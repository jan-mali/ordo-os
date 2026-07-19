#pragma once

#include <Core/PCI/Device.hpp>
#include <Drivers/Device.hpp>
#include <Drivers/Driver.hpp>

namespace Drivers::GPT {

struct GPTHeader {
	char signature[8];
	uint32_t revision;
	uint32_t header_size;
	uint32_t crc32;
	uint32_t reserved;
	uint64_t current_lba;
	uint64_t backup_lba;
	uint64_t first_usable_lba;
	uint64_t last_usable_lba;
	uint8_t disk_guid[16];
	uint64_t partition_table_start_lba;
	uint32_t partition_entries;
	uint32_t partition_entry_size;
	uint32_t partition_entries_crc32;
} __attribute__((packed));

struct GPTPartitionEntry {
	uint8_t type_guid[16];
	uint8_t partition_guid[16];
	uint64_t first_lba;
	uint64_t last_lba;
	uint64_t attributes;
	uint16_t partition_name[36];
} __attribute__((packed));

class GPTDriver : public Driver {
public:
	virtual ~GPTDriver() = default;
	bool probe_pci(PCI::Device&) override {
		// No support for PCI devices
		return false;
	}

	bool attach(PCI::Device&) override {
		// No support for PCI devices
		return false;
	}

	bool probe_device(Device&) override;
	bool attach(Device&) override;

	const char* name() const override {
		return "GPTDriver";
	}

private:
};

}
