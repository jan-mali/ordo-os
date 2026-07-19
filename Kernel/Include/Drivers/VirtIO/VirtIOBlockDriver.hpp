#pragma once

#include <Drivers/Driver.hpp>

namespace Drivers::VirtIO {

class VirtIOBlockDriver : public Driver {
public:
	bool probe_pci(PCI::Device& pci_device) override;
	bool attach(PCI::Device& pci_device) override;

	bool probe_device(Device&) override {
		// No support for software devices
		return false;
	}

	bool attach(Device&) override {
		// No support for software devices
		return false;
	}

	const char* name() const override {
		return "VirtIOBlockDriver";
	}

private:
};

}
