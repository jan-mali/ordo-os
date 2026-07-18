#include <Core/MM/PMM.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/PCI/Device.hpp>
#include <Core/Panic.hpp>

namespace PCI {

void* Device::map_bar(int bar_number) {
	BAR& bar = bars[bar_number];

	switch (bar.type) {
	case PCI::BAR::Type::MMIO32:
	case PCI::BAR::Type::MMIO64: {
		if (!Paging::kernel_address_space.map_pci_bar_to_hhdm(bar.address, bar.size)) {
			panic("Failed to map PCI BAR\n");
		}

		return reinterpret_cast<void*>(pmm.translate_to_hhdm(bar.address));
	}
	default: {
		panic("PCI::Device: Cannot map BAR%d for device %04x:%04x @ %02x:%02x.%d because as it is not MMIO.\n", bar_number, vendor, device_id, bus, device, function);
	}
	}
}

uint8_t Device::capability_offset(CapabilityID id) {
	for (size_t i = 0; i < capabilities.size(); i++) {
		if (capabilities[i].id == static_cast<uint8_t>(id)) {
			return capabilities[i].offset;
		}
	}

	return 0;
}

bool Device::has_capability(CapabilityID id) {
	return capability_offset(id);
}

}
