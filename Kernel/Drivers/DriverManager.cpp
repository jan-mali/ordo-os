#include <Drivers/DriverManager.hpp>
#include <Drivers/GPT/GPTDriver.hpp>
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

			Logger::log_warning("DriverManager: Failed to attach PCI driver %s to device %02x:%02x.%d\n", drivers[i]->name(), device.bus, device.device, device.function);
		}
	}
}

void DriverManager::probe_device(Device& device) {
	for (size_t i = 0; i < drivers.size(); i++) {
		if (drivers[i]->probe_device(device)) {
			bool success = drivers[i]->attach(device);
			if (success) {
				return;
			}

			// TODO: Get more information
			Logger::log_warning("DriverManager: Failed to attach driver %s to the device\n", drivers[i]->name());
		}
	}
}

void register_all_drivers() {
	DriverManager::register_driver(new VirtIO::VirtIOBlockDriver());
	DriverManager::register_driver(new GPT::GPTDriver());
}

}
