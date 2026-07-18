#include <Core/MM/KernelVMM.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/memory.hpp>
#include <Drivers/VirtIO/VirtQueue.hpp>
#include <Logger/Logger.hpp>

namespace Drivers::VirtIO {

struct VirtQueueMemory {
	uint64_t physical_address;
	void* virtual_address;
};

static VirtQueueMemory allocate_queue_memory(uint64_t bytes) {
	uint64_t allocation_pages = (bytes + Paging::PAGE_SIZE - 1) / Paging::PAGE_SIZE;

	void* address = kernel_vmm.allocate_pages(allocation_pages, true);
	uint64_t physical = Paging::kernel_address_space.translate(reinterpret_cast<uint64_t>(address));

	return {
		.physical_address = physical,
		.virtual_address = address
	};
}

bool VirtQueue::initialize(volatile CommonConfig* config, uint16_t index) {
	queue_index = index;

	config->queue_select = index;

	size = config->queue_size;

	if (size == 0) {
		Logger::log_error("VirtQueue: Queue does not exist\n");
		return false;
	}

	Logger::log_info("VirtQueue: queue %d size=%d\n", index, size);

	uint64_t descriptor_bytes = sizeof(VirtQueueDescriptor) * size;
	uint64_t available_bytes = sizeof(uint16_t) * 3 + sizeof(uint16_t) * size;
	uint64_t used_bytes = sizeof(uint16_t) * 2 + sizeof(VirtQueueUsedElement) * size;

	VirtQueueMemory descriptor_memory = allocate_queue_memory(descriptor_bytes);
	VirtQueueMemory available_memory = allocate_queue_memory(available_bytes);
	VirtQueueMemory used_memory = allocate_queue_memory(used_bytes);

	descriptors = reinterpret_cast<VirtQueueDescriptor*>(descriptor_memory.virtual_address);
	available = reinterpret_cast<VirtQueueAvailable*>(available_memory.virtual_address);
	used = reinterpret_cast<VirtQueueUsed*>(used_memory.virtual_address);

	descriptors_physical = descriptor_memory.physical_address;
	available_physical = available_memory.physical_address;
	used_physical = used_memory.physical_address;

	memset(descriptors, 0, descriptor_bytes);
	memset(available, 0, available_bytes);
	memset(used, 0, used_bytes);

	available_index = 0;
	used_index = 0;

	config->queue_desc = descriptors_physical;
	config->queue_driver = available_physical;
	config->queue_device = used_physical;

	for (uint16_t i = 0; i < size; i++) {
		free_descriptors.push_back(i);
	}

	config->queue_enable = 1;

	Logger::log_info(
		"VirtQueue: desc=%016llX avail=%016llX used=%016llX\n",
		descriptors_physical,
		available_physical,
		used_physical
	);

	return true;
}

int VirtQueue::allocate_descriptor() {
	if (free_descriptors.empty()) {
		return -1;
	}

	uint16_t id = free_descriptors.remove_front();

	return id;
}

void VirtQueue::free_descriptor(uint16_t index) {
	free_descriptors.push_back(index);
}

}
