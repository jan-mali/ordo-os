#pragma once

#include <Core/PCI/Device.hpp>
#include <Drivers/Driver.hpp>
#include <Utility/Vector.hpp>

namespace Drivers {

class DriverManager {
public:
	static void register_driver(Driver* driver);
	static void probe_pci(PCI::Device& device);

private:
	static Vector<Driver*> drivers;
};

void register_all_drivers();

}
