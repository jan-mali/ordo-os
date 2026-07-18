#include <Core/PCI/ConfigSpace.hpp>
#include <Core/PCI/Device.hpp>
#include <Drivers/DeviceManager.hpp>
#include <Drivers/VirtIO/VirtIOBlockDevice.hpp>
#include <Drivers/VirtIO/VirtIOBlockDriver.hpp>
#include <Drivers/VirtIO/VirtIOCapability.hpp>
#include <Logger/Logger.hpp>

namespace Drivers::VirtIO {

bool VirtIOBlockDriver::probe_pci(PCI::Device& pci_device) {
	// TODO: Maybe add support for legacy devices in the FAR FAR future :^D
	return pci_device.vendor == 0x1AF4 && pci_device.device_id == 0x1042;
}

bool VirtIOBlockDriver::attach(PCI::Device& pci_device) {
	Logger::log_info("VirtIO: Attempting to attach a modern VirtIO PCI Block Device\n");

	// Parse vendor-specific capabilities

	// ConfigurationRegion.bar defaults to 0xFF (invalid bar)
	ConfigurationRegion common_region;
	ConfigurationRegion notify_region;
	ConfigurationRegion isr_region;
	ConfigurationRegion device_region;

	for (PCI::Capabability& cap : pci_device.capabilities) {
		if (cap.id != static_cast<uint8_t>(PCI::CapabilityID::VendorSpecific)) {
			continue;
		}

		uint8_t type = PCI::ConfigurationSpace::read8(pci_device.bus, pci_device.device, pci_device.function, cap.offset + 3);
		uint8_t bar = PCI::ConfigurationSpace::read8(pci_device.bus, pci_device.device, pci_device.function, cap.offset + 4);
		uint32_t offset = PCI::ConfigurationSpace::read32(pci_device.bus, pci_device.device, pci_device.function, cap.offset + 8);
		uint32_t length = PCI::ConfigurationSpace::read32(pci_device.bus, pci_device.device, pci_device.function, cap.offset + 12);

		Logger::log_info("VirtIO cap: type=%d BAR=%d offset=%X size=%X\n", type, bar, offset, length);

		switch (type) {
		case VIRTIO_PCI_CAP_COMMON_CFG:
			// The VirtIO spec mandates that we take the first value that is valid
			if (common_region.bar > 5 && bar <= 5) {
				common_region.bar = bar;
				common_region.offset = offset;
				common_region.length = length;
			}
			break;
		case VIRTIO_PCI_CAP_NOTIFY_CFG:
			// The VirtIO spec mandates that we take the first value that is valid
			if (notify_region.bar > 5 && bar <= 5) {
				notify_region.bar = bar;
				notify_region.offset = offset;
				notify_region.length = length;
			}
			break;
		case VIRTIO_PCI_CAP_ISR_CFG:
			// The VirtIO spec mandates that we take the first value that is valid
			if (isr_region.bar > 5 && bar <= 5) {
				isr_region.bar = bar;
				isr_region.offset = offset;
				isr_region.length = length;
			}
			break;
		case VIRTIO_PCI_CAP_DEVICE_CFG:
			// The VirtIO spec mandates that we take the first value that is valid
			if (device_region.bar > 5 && bar <= 5) {
				device_region.bar = bar;
				device_region.offset = offset;
				device_region.length = length;
			}
			break;
		}
	}

	if (common_region.bar > 5) {
		Logger::log_error("VirtIO: common_bar wasn't provided.\n");
		return false;
	}

	if (notify_region.bar > 5) {
		Logger::log_error("VirtIO: notify_bar wasn't provided.\n");
		return false;
	}

	if (isr_region.bar > 5) {
		Logger::log_error("VirtIO: isr_bar wasn't provided.\n");
		return false;
	}

	if (device_region.bar > 5) {
		Logger::log_error("VirtIO: device_bar wasn't provided.\n");
		return false;
	}

	volatile uint8_t* common_region_base = reinterpret_cast<volatile uint8_t*>(pci_device.map_bar(common_region.bar));
	if (!common_region_base) {
		Logger::log_error("VirtIO: Failed to map the common_bar.\n");
		return false;
	}

	volatile uint8_t* notify_region_base = reinterpret_cast<volatile uint8_t*>(pci_device.map_bar(notify_region.bar));
	if (!notify_region_base) {
		Logger::log_error("VirtIO: Failed to map the notify_bar.\n");
		return false;
	}

	volatile uint8_t* isr_region_base = reinterpret_cast<volatile uint8_t*>(pci_device.map_bar(isr_region.bar));
	if (!isr_region_base) {
		Logger::log_error("VirtIO: Failed to map the isr_bar.\n");
		return false;
	}

	volatile uint8_t* device_region_base = reinterpret_cast<volatile uint8_t*>(pci_device.map_bar(device_region.bar));
	if (!device_region_base) {
		Logger::log_error("VirtIO: Failed to map the device_bar.\n");
		return false;
	}

	common_region.bar_base = common_region_base;
	common_region.address = common_region_base + common_region.offset;

	notify_region.bar_base = notify_region_base;
	notify_region.address = notify_region_base + notify_region.offset;

	isr_region.bar_base = isr_region_base;
	isr_region.address = isr_region_base + isr_region.offset;

	device_region.bar_base = device_region_base;
	device_region.address = device_region_base + device_region.offset;

	ConfigurationPointers pointers {
		common_region,
		notify_region,
		isr_region,
		device_region
	};

	VirtIOBlockDevice* device = new VirtIOBlockDevice(pointers, &pci_device);

	if (!device->initialize()) {
		Logger::log_error("VirtIO: VirtIOBlockDevice initialization failed.\n");
		delete device;
		return false;
	}

	DeviceManager::register_device(device);

	return true;
}

}
