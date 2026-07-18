#pragma once

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

	void* map_bar(int bar_number);
};

}
