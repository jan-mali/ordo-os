#pragma once

#include <Core/PCI/Device.hpp>
#include <Drivers/BlockDevice.hpp>
#include <Drivers/VirtIO/VirtIOCapability.hpp>
#include <Drivers/VirtIO/VirtQueue.hpp>
#include <cstdint>

namespace Drivers::VirtIO {

class VirtIOBlockDevice : public BlockDevice {
public:
	VirtIOBlockDevice(ConfigurationPointers config, PCI::Device* pci_device)
		: config(config)
		, pci_device(pci_device) {}

	bool read(uint64_t lba, uint64_t count, uint8_t* buffer) override;
	bool write(uint64_t lba, uint64_t count, uint8_t* buffer) override;

	bool initialize();

private:
	ConfigurationPointers config;
	PCI::Device* pci_device;

	VirtQueue* virtqueue;
};

}
