#include <Arch/x86_64/Interrupts.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>
#include <cstdarg>

void panic(const char* fmt, ...) {
	Interrupts::disable();

	Logger::log_critical("KERNEL PANIC! Halting...\n");

	va_list args;
	va_start(args, fmt);

	Logger::vlog(Logger::LogLevel::Critical, fmt, args);

	va_end(args);

	for (;;) {
		Interrupts::disable();
		asm volatile("hlt");
	}
}
