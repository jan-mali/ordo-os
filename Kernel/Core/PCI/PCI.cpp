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
			for (int function = 0; function < 8; function++) {
				uint16_t vendor = ConfigurationSpace::read16(bus, device, function, 0);

				if (vendor == 0xFFFF) {
					// Device doesn't exist, skip
					continue;
				}

				uint16_t device_id = ConfigurationSpace::read16(bus, device, function, 0x2);

				uint8_t prog_if = ConfigurationSpace::read8(bus, device, function, 0x9);
				uint8_t subclass = ConfigurationSpace::read8(bus, device, function, 0xA);
				uint8_t class_code = ConfigurationSpace::read8(bus, device, function, 0xB);

				devices.push_back({ .bus = static_cast<uint8_t>(bus), .device = static_cast<uint8_t>(device), .function = static_cast<uint8_t>(function), .vendor = vendor, .device_id = device_id, .class_code = class_code, .subclass = subclass, .prog_if = prog_if });

				Logger::log_info("- %02X:%02X.%d: vendor=%04X, device=%04X cl:sub:if=%02X:%02X:%02X\n", bus, device, function, vendor, device, class_code, subclass, prog_if);
			}
		}
	}
}

}
