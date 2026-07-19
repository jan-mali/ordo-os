#include <Drivers/Device.hpp>
#include <Drivers/DeviceManager.hpp>
#include <Drivers/DriverManager.hpp>
#include <Utility/Vector.hpp>

namespace Drivers {

Vector<Device*> DeviceManager::devices;

void DeviceManager::register_device(Device* device) {
	devices.push_back(device);
	DriverManager::probe_device(*device);
}

}
