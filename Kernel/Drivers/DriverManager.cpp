#include <Drivers/DriverManager.hpp>
#include <Drivers/VirtIO/VirtIOBlockDriver.hpp>
#include <Logger/Logger.hpp>
#include <Utility/Vector.hpp>

namespace Drivers {

Vector<Driver*> DriverManager::drivers;

void DriverManager::register_driver(Driver* driver) {
	drivers.push_back(driver);
}

void DriverManager::probe_pci(PCI::Device& device) {
	// Should this function return a bool to indicate success?

	for (size_t i = 0; i < drivers.size(); i++) {
		if (drivers[i]->probe_pci(device)) {
			bool success = drivers[i]->attach(device);
			if (success) {
				return;
			}

			Logger::log_warning("DriverManager: Failed to attach PCI driver to device %02x:%02x.%d\n", device.bus, device.device, device.function);
		}
	}
}

void register_all_drivers() {
	DriverManager::register_driver(new VirtIO::VirtIOBlockDriver());
}

}
