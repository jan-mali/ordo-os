#pragma once

#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <cstdint>

constexpr int MAX_STACK_UNWIND_LENGTH = 20;

struct StackFrame {
	StackFrame* rbp;
	uint64_t rip;
};

static void stack_trace() {
	StackFrame* frame;
	StackFrame* frame2;

	asm volatile(
		"mov %%rbp, %0"
		: "=r"(frame)
	);

	frame2 = frame;

	Logger::log_critical("Stack trace:\n");

	for (int i = 0; frame && i < MAX_STACK_UNWIND_LENGTH; i++) {
		Logger::log_critical("\t#%d %016llX\n", i, frame->rip);

		if (frame->rbp <= frame)
			break;

		frame = frame->rbp;
	}

	Logger::log_debug("x86_64-elf-addr2line -e Build/kernel.elf -piasfC");

	for (int i = 0; frame2 && i < MAX_STACK_UNWIND_LENGTH; i++) {
		Logger::kprintf(
			" %016llX",
			frame2->rip
		);

		if (frame2->rbp <= frame2)
			break;

		frame2 = frame2->rbp;
	}

	Logger::kprintf("\n");
}
