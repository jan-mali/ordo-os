#pragma once

#include <Core/PCI/Device.hpp>

namespace Drivers {

class Driver {
public:
	virtual ~Driver() = default;
	virtual bool probe_pci(PCI::Device&) = 0;
	virtual bool attach(PCI::Device&) = 0;

private:
};

}
