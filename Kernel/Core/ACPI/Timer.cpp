#include <Core/ACPI/Timer.hpp>

namespace ACPI::Timer {

constexpr uint32_t LVT_TIMER = 0x320;
constexpr uint32_t INITIAL_COUNT = 0x380;
constexpr uint32_t CURRENT_COUNT = 0x390;
constexpr uint32_t DIVIDE_CONFIG = 0x3E0;

constexpr uint32_t LAPIC_TIMER_ONE_SHOT = 0 << 17;
constexpr uint32_t LAPIC_TIMER_PERIODIC = 1 << 17;
constexpr uint32_t LAPIC_TIMER_TSC_DEADLINE = 2 << 17; // if supported

constexpr uint32_t LAPIC_LVT_MASKED = 1 << 16;

constexpr uint32_t LAPIC_DIVIDE_BY_2 = 0b0000;
constexpr uint32_t LAPIC_DIVIDE_BY_4 = 0b0001;
constexpr uint32_t LAPIC_DIVIDE_BY_8 = 0b0010;
constexpr uint32_t LAPIC_DIVIDE_BY_16 = 0b0011;
constexpr uint32_t LAPIC_DIVIDE_BY_32 = 0b1000;
constexpr uint32_t LAPIC_DIVIDE_BY_64 = 0b1001;
constexpr uint32_t LAPIC_DIVIDE_BY_128 = 0b1010;
constexpr uint32_t LAPIC_DIVIDE_BY_1 = 0b1011;

static void init_lapic_timer(uint8_t vector, uint32_t initial_count) {
	ACPI::Interrupts::interrupt_manager->lapic().write32(DIVIDE_CONFIG, LAPIC_DIVIDE_BY_16);
	ACPI::Interrupts::interrupt_manager->lapic().write32(LVT_TIMER, vector | LAPIC_TIMER_PERIODIC); // IRQ 32
	ACPI::Interrupts::interrupt_manager->lapic().write32(INITIAL_COUNT, initial_count);
}

void init() {
	init_lapic_timer(32, 5'000'000);
}

}
