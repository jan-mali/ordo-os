#include <Arch/x86_64/IO.hpp>
#include <Core/PCI/ConfigSpace.hpp>

namespace PCI::ConfigurationSpace {

constexpr uint16_t CONFIG_ADDRESS = 0xCF8;
constexpr uint16_t CONFIG_DATA = 0xCFC;

// TODO: Support ECAM

uint8_t read8(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
	uint32_t bus32 = static_cast<uint32_t>(bus);
	uint32_t dev32 = static_cast<uint32_t>(dev);
	uint32_t func32 = static_cast<uint32_t>(func);

	uint32_t address = static_cast<uint32_t>((bus32 << 16) | (dev32 << 11) | (func32 << 8) | (offset & 0xFC) | 0x80000000);

	IO::outl(CONFIG_ADDRESS, address);

	uint8_t value = static_cast<uint16_t>((IO::inl(CONFIG_DATA) >> ((offset & 3) * 8)) & 0xFF);

	return value;
}

uint16_t read16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
	uint32_t bus32 = static_cast<uint32_t>(bus);
	uint32_t dev32 = static_cast<uint32_t>(dev);
	uint32_t func32 = static_cast<uint32_t>(func);

	uint32_t address = static_cast<uint32_t>((bus32 << 16) | (dev32 << 11) | (func32 << 8) | (offset & 0xFC) | 0x80000000);

	IO::outl(CONFIG_ADDRESS, address);

	uint16_t value = static_cast<uint16_t>((IO::inl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);

	return value;
}

uint32_t read32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
	uint32_t bus32 = static_cast<uint32_t>(bus);
	uint32_t dev32 = static_cast<uint32_t>(dev);
	uint32_t func32 = static_cast<uint32_t>(func);

	uint32_t address = static_cast<uint32_t>((bus32 << 16) | (dev32 << 11) | (func32 << 8) | (offset & 0xFC) | 0x80000000);

	IO::outl(CONFIG_ADDRESS, address);

	return IO::inl(CONFIG_DATA);
}

}
