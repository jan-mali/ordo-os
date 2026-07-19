#include <Arch/x86_64/IDT.hpp>
#include <Core/ACPI/Interrupts.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/PCI/MSIX.hpp>
#include <Drivers/VirtIO/VirtIOBlockDevice.hpp>
#include <Drivers/VirtIO/VirtIOCapability.hpp>
#include <Drivers/VirtIO/VirtQueue.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>

namespace Drivers::VirtIO {

bool VirtIOBlockDevice::read(uint64_t lba, uint64_t count, uint8_t* buffer) {
	return execute_request(VIRTIO_BLK_T_IN, lba, count, buffer);
}

bool VirtIOBlockDevice::write(uint64_t lba, uint64_t count, uint8_t* buffer) {
	return execute_request(VIRTIO_BLK_T_OUT, lba, count, buffer);
}

bool VirtIOBlockDevice::execute_request(uint32_t type, uint64_t lba, uint64_t count, volatile uint8_t* buffer) {
	BlockRequest req;
	req.header.type = type;
	req.header.reserved = 0;
	req.header.sector = lba; // VirtIO treats sectors as 512 bytes

	// 1. Critical section for VirtQueue ring mutation
	device_lock.lock();

	int head = virtqueue->allocate_descriptor();
	int data_desc = virtqueue->allocate_descriptor();
	int status_desc = virtqueue->allocate_descriptor();

	if (head < 0 || data_desc < 0 || status_desc < 0) {
		if (head >= 0)
			virtqueue->free_descriptor(head);
		if (data_desc >= 0)
			virtqueue->free_descriptor(data_desc);
		if (status_desc >= 0)
			virtqueue->free_descriptor(status_desc);
		device_lock.unlock();
		return false;
	}

	// Head Descriptor: Command Header (Read-Only for device)
	virtqueue->descriptors[head].address = Paging::kernel_address_space.translate(reinterpret_cast<uint64_t>(&req.header));
	virtqueue->descriptors[head].length = sizeof(req.header);
	virtqueue->descriptors[head].flags = VIRTQ_DESC_F_NEXT;
	virtqueue->descriptors[head].next = data_desc;

	// Data Descriptor: Target Data Buffer
	virtqueue->descriptors[data_desc].address = Paging::kernel_address_space.translate(reinterpret_cast<uint64_t>(buffer));
	virtqueue->descriptors[data_desc].length = count * 512;
	virtqueue->descriptors[data_desc].flags = VIRTQ_DESC_F_NEXT | (type == VIRTIO_BLK_T_IN ? VIRTQ_DESC_F_WRITE : 0);
	virtqueue->descriptors[data_desc].next = status_desc;

	// Status Descriptor: Single byte response (Write-Only for device)
	virtqueue->descriptors[status_desc].address = Paging::kernel_address_space.translate(reinterpret_cast<uint64_t>((void*)&req.status));
	virtqueue->descriptors[status_desc].length = 1;
	virtqueue->descriptors[status_desc].flags = VIRTQ_DESC_F_WRITE;
	virtqueue->descriptors[status_desc].next = 0;

	// Save tracking pointer bound to the head index
	outstanding_requests[head] = &req;

	// Place into the available ring
	uint16_t avail_slot = virtqueue->available_index % virtqueue->size;
	virtqueue->available->ring[avail_slot] = head;
	virtqueue->available_index++;

	// Ensure data writes are visible to the CPU memory bus before index advance
	__asm__ volatile("" ::: "memory");
	virtqueue->available->index = virtqueue->available_index;

	// Notify the hardware via the MMIO/PIO Notification capability base
	uintptr_t notify_address = reinterpret_cast<uintptr_t>(config.notify.address) + virtqueue->notify_offset * notify_offset_multiplier;
	*(volatile uint16_t*)notify_address = virtqueue->queue_index;

	device_lock.unlock();

	// 2. Thread Sleep Section
	req.lock.acquire(); // Acquire the local RawSpinlock
	while (!req.completed) {
		// Drop local lock, yield CPU, and re-acquire upon wakeup
		req.wq.sleep(req.lock);
	}
	req.lock.release();

	// 3. Post-completion descriptor reclamation
	device_lock.lock();
	virtqueue->free_descriptor(head);
	virtqueue->free_descriptor(data_desc);
	virtqueue->free_descriptor(status_desc);
	device_lock.unlock();

	return req.status == 0; // VirtIO success state is 0 (VIRTIO_BLK_S_OK)
}

uint64_t VirtIOBlockDevice::sector_count() {
	volatile BlockDeviceConfig* device_config = reinterpret_cast<volatile BlockDeviceConfig*>(config.device.address);

	return device_config->capacity;
}

bool VirtIOBlockDevice::initialize() {
	// Spec link: https://docs.oasis-open.org/virtio/virtio/v1.1/cs01/virtio-v1.1-cs01.html#x1-92001r90

	volatile CommonConfig* common_config = reinterpret_cast<volatile CommonConfig*>(config.common.address);

	// 1. Reset the device.

	common_config->device_status = 0;
	while (common_config->device_status != 0) {
		asm volatile("pause");
	}

	// 2. Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.

	common_config->device_status = static_cast<uint8_t>(DeviceStatusFlags::Acknowledge);

	// 3. Set the DRIVER status bit: the guest OS knows how to drive the device.

	common_config->device_status |= static_cast<uint8_t>(DeviceStatusFlags::Driver);

	// 4. Read device feature bits, and write the subset of feature bits understood by the OS and driver to the device. During this step the driver MAY read (but MUST NOT write) the device-specific configuration fields to check that it can support the device before accepting it.

	common_config->device_feature_select = 0;
	uint64_t device_features_low = common_config->device_feature;

	common_config->device_feature_select = 1;
	uint64_t device_features_high = common_config->device_feature;

	uint64_t device_features = (device_features_high << 32) | device_features_low;

	// We are dumb, don't accept any features
	uint64_t driver_features = device_features & 0;

	common_config->driver_feature_select = 0;
	common_config->driver_feature = driver_features & 0xFFFFFFFF;

	common_config->driver_feature_select = 1;
	common_config->driver_feature = driver_features >> 32;

	// 5. Set the FEATURES_OK status bit. The driver MUST NOT accept new feature bits after this step.

	common_config->device_status |= static_cast<uint8_t>(DeviceStatusFlags::FeaturesOk);

	// 6. Re-read device status to ensure the FEATURES_OK bit is still set: otherwise, the device does not support our subset of features and the device is unusable.
	if (!(common_config->device_status & static_cast<uint8_t>(DeviceStatusFlags::FeaturesOk))) {
		Logger::log_error("VirtIO: Device rejected features\n");

		// Bail politely
		common_config->device_status |= static_cast<uint8_t>(DeviceStatusFlags::Failed);

		return false;
	}

	Logger::log_info("VirtIO: Feature negotation successful.\n");

	// 7. Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup, reading and possibly writing the device’s virtio configuration space, and population of virtqueues.

	// Configure the VirtQueues

	uint16_t queue_count = common_config->num_queues;
	if (queue_count == 0) {
		Logger::log_error("VirtIO: BlockDevice has no virtqueues\n");

		// Bail politely
		common_config->device_status |= static_cast<uint8_t>(DeviceStatusFlags::Failed);

		return false;
	}

	for (int i = 0; i < queue_count; i++) {
		uint16_t queue_size = common_config->queue_size;

		if (queue_size == 0) {
			continue;
		}

		VirtQueue* queue = new VirtQueue();
		queue->initialize(common_config, i);

		virtqueue = queue;

		// This is set up later in this function
		common_config->queue_msix_vector = 0;

		break;
	}

	uint8_t irq_vector = Interrupts::IDT::allocate_dynamic_vector();
	Interrupts::IDT::set_dynamic_handler(irq_vector, this, [](void* object, Registers* regs) {
		VirtIOBlockDevice* obj = static_cast<VirtIOBlockDevice*>(object);
		obj->handle_interrupt(regs);
	});

	// Queue IRQs
	PCI::MSIX::setup_vector(*pci_device, 0, irq_vector);
	// Config IRQs
	PCI::MSIX::setup_vector(*pci_device, 1, irq_vector);

	PCI::MSIX::enable(*pci_device);

	common_config->msix_config = 1;

	outstanding_requests = new BlockRequest*[virtqueue->size];

	// 8. Set the DRIVER_OK status bit. At this point the device is “live”.

	common_config->device_status |= static_cast<uint8_t>(DeviceStatusFlags::DriverOk);

	Logger::log_info("VirtIO: Successfully initialized the block device\n");

	return true;
}

void VirtIOBlockDevice::handle_interrupt(Registers*) {
	ACPI::Interrupts::interrupt_manager->lapic().eoi();

	uint8_t isr_status = *config.isr.address;
	if (!(isr_status & 1)) {
		return;
	}

	device_lock.lock();

	// Process all descriptors completed by the device since our last iteration
	while (virtqueue->used_index != virtqueue->used->index) {
		uint16_t slot = virtqueue->used_index % virtqueue->size;
		uint32_t head_id = virtqueue->used->ring[slot].id;

		BlockRequest* req = outstanding_requests[head_id];
		if (req) {
			// Detach tracking immediately so the slot can be reused safely
			outstanding_requests[head_id] = nullptr;

			// Protect thread state configuration change
			req->lock.acquire();
			req->completed = true;

			req->wq.wake_all();

			req->lock.release();
		}

		virtqueue->used_index++;
	}

	device_lock.unlock();
}

}
