#include <Core/memory.hpp>
#include <Drivers/BlockDevice.hpp>
#include <Drivers/DeviceManager.hpp>
#include <Drivers/GPT/GPTDriver.hpp>
#include <Drivers/GPT/GPTPartitionDevice.hpp>
#include <Logger/Logger.hpp>
#include <cstdint>

namespace Drivers::GPT {

bool GPTDriver::probe_device(Device& device) {
	// Check for a BlockDevice
	if (auto* block_device = dynamic_cast<BlockDevice*>(&device)) {
		uint32_t sector_size = block_device->sector_size();

		uint8_t* lba1 = new uint8_t[sector_size];
		GPTHeader* gpt_header = reinterpret_cast<GPTHeader*>(lba1);

		if (!block_device->read(1, 1, lba1)) {
			goto failure;
		}

		if (memcmp(gpt_header->signature, "EFI PART", 8) != 0) {
			goto failure;
		}

		// TODO: Validate CRC32 :)
		if (false) {
			goto failure;
		}

		delete[] lba1;
		return true;

	failure:
		delete[] lba1;
	}

	return false;
}

bool GPTDriver::attach(Device& device) {
	auto* block_device = dynamic_cast<BlockDevice*>(&device);
	uint32_t sector_size = block_device->sector_size();

	uint8_t* lba1 = new uint8_t[sector_size];
	GPTHeader* gpt_header = reinterpret_cast<GPTHeader*>(lba1);

	if (!block_device->read(1, 1, lba1)) {
		Logger::log_error("GPTDriver: Failed to read LBA1 from the device\n");
		delete[] lba1;
		return false;
	}

	Logger::log_info("GPTDriver: Found device with GUID %g\n", gpt_header->disk_guid);

	uint64_t partition_table_start_lba = gpt_header->partition_table_start_lba;
	uint32_t partition_entries = gpt_header->partition_entries;
	uint32_t partition_entry_size = gpt_header->partition_entry_size;

	uint64_t partition_table_bytes = partition_entries * partition_entry_size;
	uint64_t partition_table_sectors = (partition_table_bytes + sector_size - 1) / sector_size;

	uint8_t* partition_table = new uint8_t[partition_table_bytes];

	if (!block_device->read(partition_table_start_lba, partition_table_sectors, partition_table)) {
		Logger::log_error("GPTDriver: Failed to read %d sectors from the device starting at LBA%d\n", partition_table_sectors, partition_table_start_lba);
		delete[] partition_table;
		delete[] lba1;
		return false;
	}

	// TODO: Maybe validate the CRC32 :)

	for (size_t i = 0; i < partition_entries; i++) {
		GPTPartitionEntry* entry = reinterpret_cast<GPTPartitionEntry*>(&partition_table[i * partition_entry_size]);

		bool unused = true;

		for (int j = 0; j < 16; j++) {
			if (entry->type_guid[j] != 0) {
				unused = false;
				break;
			}
		}

		if (unused)
			continue;

		Logger::log_info("GPTDriver: Partition %d. GUID=%g, Type=%g, Name=%w\n", i, entry->partition_guid, entry->type_guid, entry->partition_name);

		GPTPartitionDevice* partition_device = new GPTPartitionDevice(block_device, *entry);

		DeviceManager::register_device(partition_device);
	}

	delete[] partition_table;
	delete[] lba1;

	return true;
}

};
