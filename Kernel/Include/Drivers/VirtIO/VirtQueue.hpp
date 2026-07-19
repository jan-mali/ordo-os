#pragma once

#include <Drivers/VirtIO/VirtIOCapability.hpp>
#include <Utility/Vector.hpp>
#include <cstdint>

namespace Drivers::VirtIO {

class VirtIOBlockDevice;

struct VirtQueueDescriptor {
	uint64_t address;
	uint32_t length;
	uint16_t flags;
	uint16_t next;
};

struct VirtQueueAvailable {
	uint16_t flags;
	uint16_t index;
	uint16_t ring[1];
};

struct VirtQueueUsedElement {
	uint32_t id;
	uint32_t length;
};

struct VirtQueueUsed {
	uint16_t flags;
	uint16_t index;

	VirtQueueUsedElement ring[1];
};

class VirtQueue {
	friend class VirtIOBlockDevice;

public:
	bool initialize(volatile CommonConfig* config, uint16_t index);

	int allocate_descriptor();
	void free_descriptor(uint16_t index);

private:
	uint16_t size;
	uint16_t queue_index;

	VirtQueueDescriptor* descriptors;
	VirtQueueAvailable* available;
	VirtQueueUsed* used;

	uint64_t descriptors_physical;
	uint64_t available_physical;
	uint64_t used_physical;

	uint16_t available_index;
	uint16_t used_index;

	uint16_t notify_offset;

	Vector<uint16_t> free_descriptors;
};

}
