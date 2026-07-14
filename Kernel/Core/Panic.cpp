#include <Arch/x86_64/Interrupts.hpp>
#include <Arch/x86_64/StackTrace.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>
#include <cstdarg>

void panic(const char* fmt, ...) {
	Interrupts::disable();

	Logger::reset_indent();

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
