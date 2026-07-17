#pragma once
#include <Core/ACPI/MADT.hpp>
#include <Utility/Vector.hpp>

namespace ACPI::Interrupts {

struct IOAPIC {
	uint8_t id;
	uint64_t physical_address;
	uint32_t gsi_base;
	uint32_t redirection_count;

	uint32_t read32(uint32_t reg);
	void write32(uint32_t reg, uint32_t value);

	void redirect(uint32_t gsi, uint8_t vector, uint8_t destination, bool level_triggered, bool active_low, bool masked);
};

class LAPIC {
public:
	void init();
	void eoi();

private:
	volatile uint32_t* base;
};

struct CPUInfo {
	uint8_t acpi_id;
	uint32_t apic_id;
	bool enabled;
};

struct InterruptOverride {
	enum class TriggerMode : uint16_t {
		Default = 0b0000,
		Edge = 0b0100,
		Level = 0b1100,
		Reserved = 0b1000
	};

	enum class Polarity : uint16_t {
		Default = 0b00,
		High = 0b01,
		Low = 0b11,
		Reserved = 0b10
	};

	uint8_t bus;
	uint8_t irq;
	uint32_t gsi;
	TriggerMode trigger;
	Polarity polarity;

	static const char* trigger_mode_name(TriggerMode value) {
		switch (value) {
		case TriggerMode::Default:
			return "Default";
		case TriggerMode::Edge:
			return "Edge";
		case TriggerMode::Level:
			return "Level";
		case TriggerMode::Reserved:
			return "Reserved";
		}
		return "Unknown";
	}

	static const char* polarity_name(Polarity value) {
		switch (value) {
		case Polarity::Default:
			return "Default";
		case Polarity::High:
			return "High";
		case Polarity::Low:
			return "Low";
		case Polarity::Reserved:
			return "Reserved";
		}
		return "Unknown";
	}
};

class LocalAPIC {
	friend class InterruptManager;

public:
	void eoi();
	uint32_t read32(uint32_t offset);
	void write32(uint32_t offset, uint32_t value);

private:
	volatile uint32_t* mmio;
};

class InterruptManager {
public:
	InterruptManager(MADT::MADTHeader* madt);

	uint32_t irq_to_gsi(uint8_t bus, uint8_t irq);

	inline LocalAPIC& lapic() {
		return lapic_;
	}

private:
	// TODO: Support SMP in the future
	IOAPIC ioapic;
	CPUInfo cpu_info;
	LocalAPIC lapic_;

	bool dual_8259_legacy_pics_installed;

	Vector<InterruptOverride> overrides;
};

void init(MADT::MADTHeader* madt);

extern InterruptManager* interrupt_manager;

}
