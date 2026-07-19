#include <Core/ACPI/Interrupts.hpp>
#include <Core/ACPI/MADT.hpp>
#include <Core/MM/PMM.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>
#include <cstdint>

namespace ACPI::Interrupts {

InterruptManager* interrupt_manager;

uint32_t IOAPIC::read32(uint32_t reg) {
	volatile uint32_t* ioapic = reinterpret_cast<volatile uint32_t*>(pmm.translate_to_hhdm(physical_address));
	ioapic[0] = (reg & 0xff);
	return ioapic[4];
}

void IOAPIC::write32(uint32_t reg, uint32_t value) {
	volatile uint32_t* ioapic = reinterpret_cast<volatile uint32_t*>(pmm.translate_to_hhdm(physical_address));
	ioapic[0] = (reg & 0xff);
	ioapic[4] = value;
}

void IOAPIC::redirect(uint32_t gsi, uint8_t vector, uint8_t destination, bool level_triggered, bool active_low, bool masked) {
	uint32_t index = gsi - gsi_base;

	if (index > redirection_count) {
		panic("IOAPIC: Invalid GSI");
	}

	uint32_t low = 0;
	uint32_t high = 0;

	// Interrupt vector
	low |= vector;

	// Delivery mode
	// 000 = Fixed
	low |= (0b000 << 8);

	// Destination mode
	// 0 = physical APIC ID
	low |= (0 << 11);

	// Polarity
	if (active_low)
		low |= (1 << 13);

	// Trigger mode
	if (level_triggered)
		low |= (1 << 15);

	// Mask
	if (masked)
		low |= (1 << 16);

	// Destination APIC ID
	high |= ((uint32_t)destination << 24);

	// Write it
	write32(0x10 + index * 2, low);
	write32(0x11 + index * 2, high);
}

void LocalAPIC::eoi() {
	write32(0xB0, 0);
}

uint32_t LocalAPIC::read32(uint32_t offset) {
	return mmio[offset / sizeof(uint32_t)];
}

void LocalAPIC::write32(uint32_t offset, uint32_t value) {
	mmio[offset / sizeof(uint32_t)] = value;
}

InterruptManager::InterruptManager(MADT::MADTHeader* madt) {
	uint64_t lapic_address = madt->lapic_address;
	dual_8259_legacy_pics_installed = madt->flags & 1;

	uint8_t* current = reinterpret_cast<uint8_t*>(madt + 1);
	uint8_t* end = reinterpret_cast<uint8_t*>(madt) + madt->header.length;

	bool lapic_found = false;
	bool ioapic_found = false;
	int unknown_entries_encountered = 0;

	while (current < end) {
		MADT::EntryHeader* entry_header = reinterpret_cast<MADT::EntryHeader*>(current);

		switch (entry_header->type) {
		case 0: {
			MADT::EntryProcessorLAPIC* entry = reinterpret_cast<MADT::EntryProcessorLAPIC*>(entry_header);

			if (!(entry->flags & 1) && !(entry->flags & 2)) {
				// This CPU is offline & isn't online capable.
				break;
			}

			if (lapic_found) {
				Logger::log_warning("CPUInfo: Extra Local APIC found. SMP is not supported.\n");
				break;
			}

			lapic_found = true;

			cpu_info.acpi_id = entry->acpi_cpu_id;
			cpu_info.apic_id = entry->apic_id;
			cpu_info.enabled = entry->flags & 1;

			Logger::log_info("CPUInfo: ACPI %d, APIC %d, CPU is %s\n", cpu_info.acpi_id, cpu_info.apic_id, cpu_info.enabled ? "enabled" : "disabled");

			break;
		}
		case 1: {
			MADT::EntryIOAPIC* entry = reinterpret_cast<MADT::EntryIOAPIC*>(entry_header);

			if (ioapic_found)
				panic("Multiple I/O APICs found. This is NOT supported.\n");

			ioapic_found = true;

			ioapic.id = entry->id;
			ioapic.physical_address = entry->address;
			ioapic.gsi_base = entry->gsi_base;

			// Ensure the MMIO for this I/O APIC is mapped correctly into virtual memory
			Paging::kernel_address_space.map_mmio_to_hhdm(ioapic.physical_address);

			// Also get the redirection count
			uint32_t register_data = ioapic.read32(0x01);
			ioapic.redirection_count = (register_data >> 16) & 0xFF;

			Logger::log_info("I/O APIC: APIC %d, PA %016llX, GSI Base %d, Redirection Count %d\n", ioapic.id, ioapic.physical_address, ioapic.gsi_base, ioapic.redirection_count);

			break;
		}
		case 2: {
			MADT::EntryIOAPICISO* entry = reinterpret_cast<MADT::EntryIOAPICISO*>(entry_header);

			InterruptOverride iso;
			iso.bus = entry->bus;
			iso.irq = entry->irq;
			iso.gsi = entry->gsi;
			iso.polarity = static_cast<InterruptOverride::Polarity>(entry->flags & 0b0011);
			iso.trigger = static_cast<InterruptOverride::TriggerMode>(entry->flags & 0b1100);

			overrides.push_back(iso);

			Logger::log_info("InterruptOverride: Bus %d, IRQ %d, GSI %d, Polarity %s, Trigger %s\n", iso.bus, iso.irq, iso.gsi, InterruptOverride::polarity_name(iso.polarity), InterruptOverride::trigger_mode_name(iso.trigger));

			break;
		}
		case 4: {
			Logger::log_info("LAPIC NMI: Already configured by Limine\n");

			break;
		}
		default:
			Logger::log_error("ACPI: InterruptManager: Unknown MADT entry type: %d.\n", entry_header->type);
			unknown_entries_encountered++;
			break;
		}

		current += entry_header->length;
	}

	if (!lapic_found) {
		panic("ACPI: InterruptManager: Failed to find the Local APIC.\n");
	}

	if (!ioapic_found) {
		panic("ACPI: InterruptManager: Failed to find the I/O APIC.\n");
	}

	if (unknown_entries_encountered > 0) {
		panic("ACPI: InterruptManager: Encountered %d unknown MADT entries.\n", unknown_entries_encountered);
	}

	// Ensure the MMIO for the Local APIC is mapped correctly into virtual memory
	// Do it here in case the address was changed by an Local x2APIC entry
	Paging::kernel_address_space.map_mmio_to_hhdm(lapic_address);
	lapic_.mmio = reinterpret_cast<uint32_t*>(pmm.translate_to_hhdm(lapic_address));

	// Redirect the keyboard IRQ
	uint32_t keyboard_gsi = irq_to_gsi(0, 1); // Bus 0 = ISA, IRQ1 = PS/2 Keyboard
	Logger::log_debug("FIXME: InterruptManager: Get level and polarity from ISOs\n");
	ioapic.redirect(keyboard_gsi, 33, cpu_info.apic_id, false, false, false);
}

uint32_t InterruptManager::irq_to_gsi(uint8_t bus, uint8_t irq) {
	for (size_t i = 0; i < overrides.size(); i++) {
		InterruptOverride* iso = &overrides[i];
		if (iso->bus == bus && iso->irq == irq)
			return iso->gsi;
	}

	return irq;
}

void init(MADT::MADTHeader* madt) {
	interrupt_manager = new InterruptManager(madt);
}

}
