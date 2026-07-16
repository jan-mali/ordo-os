#include <Arch/x86_64/IO.hpp>
#include <Arch/x86_64/Interrupts.hpp>
#include <Logger/Logger.hpp>
#include <cstdint>

extern "C" {
void init_isr_and_lidt();
}

namespace Interrupts {

void enable() {
	asm volatile("sti");
}

void disable() {
	asm volatile("cli");
}

uint64_t store_and_disable() {
	uint64_t flags;

	asm volatile(
		"pushfq\n"
		"popq %0\n"
		"cli"
		: "=r"(flags)
		:
		: "memory"
	);

	return flags;
}

void restore(uint64_t flags) {
	asm volatile(
		"pushq %0\n"
		"popfq"
		:
		: "r"(flags)
		: "memory"
	);
}

extern "C" uint8_t idt[];

void init() {
	init_isr_and_lidt();

	Logger::log_info("IDT is initalized.\n");
}

}
