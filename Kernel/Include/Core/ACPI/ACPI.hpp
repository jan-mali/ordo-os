#pragma once

#include <cstdint>
#include <limine.h>

namespace ACPI {

struct __attribute__((packed)) XSDP {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;

	uint32_t length;
	uint64_t xsdt_address;
	uint8_t extended_checksum;
	uint8_t reserved[3];

	bool checksum_correct();
};

extern XSDP* xsdp;

struct __attribute__((packed)) SDTHeader {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;

	bool checksum_correct();
};

struct __attribute__((packed)) RSDT {
	SDTHeader header;
	uint32_t first_entry;

	uint32_t* entries();
	uint32_t entry_count();
};

struct __attribute__((packed)) XSDT {
	SDTHeader header;
	uint64_t first_entry;

	uint64_t* entries();
	uint64_t entry_count();
};

void init(limine_rsdp_response* rsdp_response);

}
