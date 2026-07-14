#include <Arch/x86_64/Interrupts.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>
#include <cstdarg>
#include <cstdint>

struct StackFrame {
	StackFrame* rbp;
	uint64_t rip;
};

void stack_trace() {
	StackFrame* frame;

	asm volatile(
		"mov %%rbp, %0"
		: "=r"(frame)
	);

	Logger::log_critical("Stack trace:\n");

	for (int i = 0; frame && i < 20; i++) {
		Logger::log_critical(
			"  #%d %X\n",
			i,
			frame->rip
		);

		frame = frame->rbp;
	}
}

void panic(const char* fmt, ...) {
	Interrupts::disable();

	Logger::log_critical("KERNEL PANIC! Halting...\n");

	va_list args;
	va_start(args, fmt);

	Logger::vlog(Logger::LogLevel::Critical, fmt, args);

	va_end(args);

	stack_trace();

	for (;;) {
		Interrupts::disable();
		asm volatile("hlt");
	}
}
