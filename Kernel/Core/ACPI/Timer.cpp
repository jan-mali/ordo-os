#include <Arch/x86_64/IO.hpp>
#include <Arch/x86_64/Interrupts.hpp>
#include <Core/ACPI/Timer.hpp>
#include <Logger/Logger.hpp>

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

constexpr uint32_t PIT_FREQUENCY = 1193182;
constexpr uint32_t PIT_HZ = 20;
constexpr uint16_t PIT_DIVISOR = PIT_FREQUENCY / PIT_HZ;

constexpr uint32_t TIMER_VECTOR = 32; // IRQ 32

uint64_t monotonic_time_ns = 0;

static void pit_start_oneshot() {
	// Channel 0
	// Access mode: lobyte/hibyte
	// Mode 0: interrupt on terminal count
	// Binary counting
	IO::outb(0x43, 0b00110000);

	IO::outb(0x40, PIT_DIVISOR & 0xFF);
	IO::outb(0x40, PIT_DIVISOR >> 8);
}

static uint16_t pit_read_counter() {
	// Latch channel 0 counter
	IO::outb(0x43, 0b00000000);

	uint8_t low = IO::inb(0x40);
	uint8_t high = IO::inb(0x40);

	return low | (high << 8);
}

void init() {
	Logger::log_info("ACPI: Initializing LAPIC Timer for calibration...\n");

	auto& lapic = ACPI::Interrupts::interrupt_manager->lapic();

	// Configure LAPIC timer, masked, one-shot
	lapic.write32(DIVIDE_CONFIG, LAPIC_DIVIDE_BY_16);
	lapic.write32(LVT_TIMER, TIMER_VECTOR | LAPIC_TIMER_ONE_SHOT | LAPIC_LVT_MASKED);
	lapic.write32(INITIAL_COUNT, 0xFFFFFFFF);

	// Start PIT countdown
	pit_start_oneshot();

	// Wait for PIT to expire
	// FIXME: This drifts a fraction of a second every minute, but it's fine for now
	while (pit_read_counter() > 10) {
		asm volatile("pause");
	}

	finish_calibration();
}

void finish_calibration() {
	auto& lapic = ACPI::Interrupts::interrupt_manager->lapic();

	uint32_t current_count = lapic.read32(CURRENT_COUNT);
	uint32_t elapsed_ticks = 0xFFFFFFFF - current_count;

	uint32_t lapic_frequency = elapsed_ticks * PIT_HZ;
	uint32_t reload = lapic_frequency / LAPIC_TIMER_HZ;

	Logger::log_info("ACPI: LAPIC frequency %d Hz, scheduler tick %d Hz, reload %d\n", lapic_frequency, LAPIC_TIMER_HZ, reload);

	// Stop old timer
	lapic.write32(INITIAL_COUNT, 0);

	// Configure periodic scheduler timer
	lapic.write32(LVT_TIMER, TIMER_VECTOR | LAPIC_TIMER_PERIODIC);

	lapic.write32(INITIAL_COUNT, reload);

	monotonic_time_ns = 0;
}

}
