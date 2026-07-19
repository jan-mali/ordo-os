#pragma once

#include <Core/PCI/Device.hpp>
#include <cstdint>

namespace PCI {

struct MSIXTableEntry {
	uint32_t address_low;
	uint32_t address_high;
	uint32_t data;
	uint32_t vector_control;
};

class MSIX {
public:
	static bool supported(Device& device);

	static bool initialize(Device& device, uint16_t vector_count);
	static bool setup_vector(Device& device, uint16_t entry, uint8_t vector);
	static void enable(Device& device);

private:
	static uint8_t capability_offset(Device& device);
};

}
