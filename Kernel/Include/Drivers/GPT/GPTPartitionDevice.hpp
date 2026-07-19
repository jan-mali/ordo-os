#pragma once

#include <Drivers/BlockDevice.hpp>
#include <Drivers/GPT/GPTDriver.hpp>

namespace Drivers::GPT {

class GPTPartitionDevice : public BlockDevice {
public:
	GPTPartitionDevice(BlockDevice* parent_device, GPTPartitionEntry partition_entry)
		: parent_device(parent_device)
		, partition_entry(partition_entry) {}

	virtual ~GPTPartitionDevice() = default;

	bool read(uint64_t lba, uint64_t count, uint8_t* buffer) override;
	bool write(uint64_t lba, uint64_t count, uint8_t* buffer) override;
	uint64_t sector_size() override;
	uint64_t sector_count() override;

private:
	BlockDevice* parent_device;
	GPTPartitionEntry partition_entry;
};

}
