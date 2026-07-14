#pragma once

#include <Core/ACPI/ACPI.hpp>
#include <cstdint>

namespace ACPI::MADT {

struct MADTHeader {
	SDTHeader header;
	uint32_t lapic_address;
	uint32_t flags;
};

struct EntryHeader {
	uint8_t type;
	uint8_t length;
};

struct EntryProcessorLAPIC {
	EntryHeader header;
	uint8_t acpi_cpu_id;
	uint8_t apic_id;
	uint32_t flags;
};

struct EntryIOAPIC {
	EntryHeader header;
	uint8_t id;
	uint8_t reserved;
	uint32_t address;
	uint32_t gsi_base;
};

struct EntryIOAPICISO {
	EntryHeader header;
	uint8_t bus;
	uint8_t irq;
	uint32_t gsi;
	uint16_t flags;
};

struct EntryIOAPICNMI {
	EntryHeader header;
	uint8_t source;
	uint8_t reserved;
	uint16_t flags;
	uint32_t gsi;
};

struct EntryLAPICNMI {
	EntryHeader header;
	uint8_t acpi_cpu_id;
	uint16_t flags;
	uint8_t lint;
};

struct EntryLAPICAddressOverride {
	EntryHeader header;
	uint16_t reserved;
	uint64_t lapic_address;
};

struct EntryProcessorX2LAPIC {
	EntryHeader header;
	uint16_t reserved;
	uint32_t cpu_local_x2apic_id;
	uint32_t flags;
	uint64_t acpi_id;
};

}
