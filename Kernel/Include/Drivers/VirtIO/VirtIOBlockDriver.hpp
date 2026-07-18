#pragma once

#include <Drivers/Driver.hpp>

namespace Drivers::VirtIO {

class VirtIOBlockDriver : public Driver {
public:
	bool probe_pci(PCI::Device& pci_device) override;
	bool attach(PCI::Device& pci_device) override;

private:
};

}
