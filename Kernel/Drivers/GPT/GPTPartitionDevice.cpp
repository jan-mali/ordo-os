#include <Drivers/GPT/GPTPartitionDevice.hpp>

namespace Drivers::GPT {

bool GPTPartitionDevice::read(uint64_t lba, uint64_t count, uint8_t* buffer) {
	if (lba > sector_count() || count > sector_count() - lba) {
		return false;
	}

	uint64_t parent_lba = partition_entry.first_lba + lba;

	return parent_device->read(parent_lba, count, buffer);
}

bool GPTPartitionDevice::write(uint64_t lba, uint64_t count, uint8_t* buffer) {
	if (lba > sector_count()) {
		return false;
	}

	uint64_t parent_lba = partition_entry.first_lba + lba;

	return parent_device->write(parent_lba, count, buffer);
}

uint64_t GPTPartitionDevice::sector_size() {
	return parent_device->sector_size();
}

uint64_t GPTPartitionDevice::sector_count() {
	// These LBAs are inclusive
	return partition_entry.last_lba - partition_entry.first_lba + 1;
}

}
