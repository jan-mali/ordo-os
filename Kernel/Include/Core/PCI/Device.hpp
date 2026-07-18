#pragma once

#include <Utility/Vector.hpp>
#include <stdint.h>

namespace PCI {

struct BAR {
	enum class Type {
		None,
		IO,
		MMIO32,
		MMIO64
	};

	Type type;

	uint64_t address;
	uint64_t size;
};

enum class CapabilityID : uint8_t {
	MSI = 0x05,
	VendorSpecific = 0x09,
	MSIX = 0x11
};

struct Capabability {
	uint8_t id;
	uint8_t offset;
} __attribute__((packed));

struct Device {
	uint8_t bus;
	uint8_t device;
	uint8_t function;

	uint16_t vendor;
	uint16_t device_id;

	uint8_t class_code;
	uint8_t subclass;
	uint8_t prog_if;

	uint8_t revision_id;
	uint8_t header_type;

	BAR bars[6];
	Vector<Capabability> capabilities;

	void* map_bar(int bar_number);
	uint8_t capability_offset(CapabilityID id);
	bool has_capability(CapabilityID id);
};

}
