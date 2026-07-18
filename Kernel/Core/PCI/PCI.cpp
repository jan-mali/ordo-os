#include <Core/PCI/ConfigSpace.hpp>
#include <Core/PCI/Device.hpp>
#include <Core/PCI/PCI.hpp>
#include <Logger/Logger.hpp>
#include <Utility/Vector.hpp>

namespace PCI {

Vector<Device> devices;

void enumerate() {
	Logger::log_info("PCI: Enumerating all devices.\n");
	for (int bus = 0; bus < 256; bus++) {
		for (int device = 0; device < 32; device++) {
			bool multifunction = false;
			for (int function = 0; function < 8; function++) {
				if (!multifunction && function > 0) {
					break;
				}

				uint16_t vendor = ConfigurationSpace::read16(bus, device, function, 0);

				if (vendor == 0xFFFF) {
					// Device doesn't exist, skip
					continue;
				}

				Device pci_device;

				uint16_t device_id = ConfigurationSpace::read16(bus, device, function, 0x2);

				uint8_t revision_id = ConfigurationSpace::read8(bus, device, function, 0x8);
				uint8_t prog_if = ConfigurationSpace::read8(bus, device, function, 0x9);
				uint8_t subclass = ConfigurationSpace::read8(bus, device, function, 0xA);
				uint8_t class_code = ConfigurationSpace::read8(bus, device, function, 0xB);

				pci_device.bus = static_cast<uint8_t>(bus);
				pci_device.device = static_cast<uint8_t>(device);
				pci_device.function = static_cast<uint8_t>(function);
				pci_device.vendor = vendor;
				pci_device.device_id = device_id;
				pci_device.class_code = class_code;
				pci_device.subclass = subclass;
				pci_device.prog_if = prog_if;
				pci_device.revision_id = revision_id;

				Logger::log_info("- %02X:%02X.%d: vendor=%04X, device=%04X cl:sub:if=%02X:%02X:%02X\n", bus, device, function, vendor, device_id, class_code, subclass, prog_if);

				uint8_t header_type = ConfigurationSpace::read8(bus, device, function, 0xE);
				pci_device.header_type = header_type;

				multifunction = header_type & 0x80;
				header_type &= 0x7F; // remove multifunction bit

				if (header_type == 0x00) {
					uint16_t command = ConfigurationSpace::read16(bus, device, function, 0x04);

					// Disable I/O & memory decode for BAR parsing
					ConfigurationSpace::write16(bus, device, function, 0x04, command & ~0x3);

					for (int i = 0; i < 6; i++) {
						uint8_t offset = 0x10 + (i * 4);

						uint32_t bar_low = ConfigurationSpace::read32(bus, device, function, offset);

						if (bar_low == 0)
							continue;

						if (bar_low & 1) {
							uint32_t original = bar_low;

							ConfigurationSpace::write32(bus, device, function, offset, 0xFFFFFFFF);
							uint32_t size = ConfigurationSpace::read32(bus, device, function, offset);
							ConfigurationSpace::write32(bus, device, function, offset, original);

							uint32_t port = bar_low & ~0x3;
							size = ~(size & ~0x3) + 1;

							pci_device.bars[i] = {
								.type = BAR::Type::IO,
								.address = port,
								.size = size
							};

							Logger::log_info("           BAR%d: I/O 0x%04X size=0x%X\n", i, port, size);
						} else {
							uint8_t bar_type = (bar_low >> 1) & 0x3;

							if (bar_type == 0x2) {
								uint32_t bar_high = ConfigurationSpace::read32(bus, device, function, offset + 4);

								uint32_t original_low = bar_low;
								uint32_t original_high = bar_high;

								ConfigurationSpace::write32(bus, device, function, offset, 0xFFFFFFFF);
								ConfigurationSpace::write32(bus, device, function, offset + 4, 0xFFFFFFFF);

								uint32_t size_low = ConfigurationSpace::read32(bus, device, function, offset);
								uint32_t size_high = ConfigurationSpace::read32(bus, device, function, offset + 4);

								ConfigurationSpace::write32(bus, device, function, offset, original_low);
								ConfigurationSpace::write32(bus, device, function, offset + 4, original_high);

								uint64_t address = ((uint64_t)bar_high << 32) | (bar_low & ~0xF);
								uint64_t size = ((uint64_t)size_high << 32) | (size_low & ~0xF);

								size = ~(size) + 1;

								pci_device.bars[i] = {
									.type = BAR::Type::MMIO64,
									.address = address,
									.size = size
								};

								Logger::log_info("           BAR%d: MMIO64 0x%016llX size=0x%llX\n", i, address, size);

								i++;
							} else {
								uint32_t original = bar_low;

								ConfigurationSpace::write32(bus, device, function, offset, 0xFFFFFFFF);
								uint32_t size = ConfigurationSpace::read32(bus, device, function, offset);
								ConfigurationSpace::write32(bus, device, function, offset, original);

								uint32_t address = bar_low & ~0xF;
								size = ~(size & ~0xF) + 1;

								pci_device.bars[i] = {
									.type = BAR::Type::MMIO32,
									.address = address,
									.size = size
								};

								Logger::log_info("           BAR%d: MMIO32 0x%08X size=0x%X\n", i, address, size);
							}
						}
					}

					// Restore the command register to original settings
					ConfigurationSpace::write16(bus, device, function, 0x04, command);

					uint16_t status = ConfigurationSpace::read16(bus, device, function, 0x06);

					if (status & (1 << 4)) {
						uint8_t capability = ConfigurationSpace::read8(bus, device, function, 0x34);

						// This is a safety check
						uint8_t capability_count = 0;

						while (capability != 0 && capability_count < 64) {
							capability_count++;
							uint8_t id = ConfigurationSpace::read8(bus, device, function, capability);
							uint8_t next = ConfigurationSpace::read8(bus, device, function, capability + 1);

							pci_device.capabilities.push_back({ .id = id, .offset = capability });

							Logger::log_info("           Capability: ID=0x%02X offset=0x%02X\n", id, capability);

							capability = next;
						}
					}
				}

				devices.push_back(pci_device);
			}
		}
	}
}

}
