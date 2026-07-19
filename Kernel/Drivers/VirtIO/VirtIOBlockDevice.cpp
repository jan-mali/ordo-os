#include <Arch/x86_64/IDT.hpp>
#include <Core/PCI/MSIX.hpp>
#include <Drivers/VirtIO/VirtIOBlockDevice.hpp>
#include <Drivers/VirtIO/VirtIOCapability.hpp>
#include <Drivers/VirtIO/VirtQueue.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>

namespace Drivers::VirtIO {

bool VirtIOBlockDevice::read(uint64_t lba, uint64_t count, uint8_t* buffer) {
	(void)lba;
	(void)count;
	(void)buffer;
	// TODO
	return false;
}

bool VirtIOBlockDevice::write(uint64_t lba, uint64_t count, uint8_t* buffer) {
	(void)lba;
	(void)count;
	(void)buffer;
	// TODO
	return false;
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
		VirtIOBlockDevice* obj = reinterpret_cast<VirtIOBlockDevice*>(object);
		obj->handle_interrupt(regs);
	});

	PCI::MSIX::setup_vector(*pci_device, 0, irq_vector);

	// For debugging only, this WILL crash
	// FIXME FIXME FIXME
	// FIXME: Config change IRQ
	PCI::MSIX::setup_vector(*pci_device, 1, 69);

	PCI::MSIX::enable(*pci_device);

	common_config->msix_config = 1;

	// 8. Set the DRIVER_OK status bit. At this point the device is “live”.

	common_config->device_status |= static_cast<uint8_t>(DeviceStatusFlags::DriverOk);

	Logger::log_info("VirtIO: Successfully initialized the block device\n");

	return true;
}

void VirtIOBlockDevice::handle_interrupt(Registers* regs) {
	Logger::log_debug("TODO: handle_interrupt :)\n");
}

}
