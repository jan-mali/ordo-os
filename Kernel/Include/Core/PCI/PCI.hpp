#pragma once

#include <Core/PCI/Device.hpp>
#include <Utility/Vector.hpp>

namespace PCI {

extern Vector<Device> devices;

void enumerate();

}
