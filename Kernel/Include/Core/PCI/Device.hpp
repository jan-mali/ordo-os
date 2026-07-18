#pragma once

#include <stdint.h>

namespace PCI {

struct Device {
	uint8_t bus;
	uint8_t device;
	uint8_t function;

	uint16_t vendor;
	uint16_t device_id;

	uint8_t class_code;
	uint8_t subclass;
	uint8_t prog_if;
};

}
