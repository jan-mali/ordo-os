#pragma once

#include <Drivers/Device.hpp>
#include <Utility/Vector.hpp>

namespace Drivers {

class DeviceManager {
public:
	static void register_device(Device* device);

private:
	static Vector<Device*> devices;
};

void register_all_drivers();

}
