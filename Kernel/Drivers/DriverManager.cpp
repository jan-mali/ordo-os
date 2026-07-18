#include <Drivers/DriverManager.hpp>
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
			drivers[i]->attach(device);
			return;
		}
	}
}

void register_all_drivers() {
	// TODO: Register all drivers here
}

}
