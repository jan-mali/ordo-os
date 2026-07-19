#include <Core/PCI/ConfigSpace.hpp>
#include <Core/PCI/Device.hpp>
#include <Core/PCI/MSIX.hpp>
#include <Logger/Logger.hpp>

namespace PCI {

bool MSIX::supported(Device& device) {
	return device.has_capability(CapabilityID::MSIX);
}

bool MSIX::initialize(Device& device, uint16_t vector_count) {
	uint8_t offset = capability_offset(device);

	if (!offset)
		return false;

	uint16_t control = ConfigurationSpace::read16(device.bus, device.device, device.function, offset + 2);

	uint16_t table_size = (control & 0x7FF) + 1;

	if (vector_count > table_size)
		vector_count = table_size;

	Logger::log_info("PCI MSI-X: table entries=%d\n", table_size);

	return true;
}

bool MSIX::setup_vector(Device& device, uint16_t entry, uint8_t vector) {
	uint8_t offset = capability_offset(device);

	uint32_t table = ConfigurationSpace::read32(device.bus, device.device, device.function, offset + 4);

	uint8_t bar = table & 0x7;
	uint32_t table_offset = table & ~0x7;

	volatile uint8_t* base = reinterpret_cast<volatile uint8_t*>(device.map_bar(bar));

	volatile MSIXTableEntry* entries = reinterpret_cast<volatile MSIXTableEntry*>(base + table_offset);

	entries[entry].address_low = 0xFEE00000;
	entries[entry].address_high = 0;
	entries[entry].data = vector;
	entries[entry].vector_control = 0;

	return true;
}

void MSIX::enable(Device& device) {
	uint8_t offset = capability_offset(device);

	uint16_t control = ConfigurationSpace::read16(device.bus, device.device, device.function, offset + 2);

	control |= (1 << 15);

	ConfigurationSpace::write16(device.bus, device.device, device.function, offset + 2, control);
}

uint8_t MSIX::capability_offset(Device& device) {
	return device.capability_offset(CapabilityID::MSIX);
}

}
