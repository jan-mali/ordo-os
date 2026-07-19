#pragma once

#include <Core/PCI/Device.hpp>
#include <Drivers/Device.hpp>

namespace Drivers {

class Driver {
public:
	virtual ~Driver() = default;
	virtual bool probe_pci(PCI::Device&) = 0;
	virtual bool attach(PCI::Device&) = 0;

	virtual bool probe_device(Device&) = 0;
	virtual bool attach(Device&) = 0;

	virtual const char* name() const = 0;

private:
};

}
