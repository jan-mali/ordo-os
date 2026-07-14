#include <Arch/x86_64/IO.hpp>
#include <Arch/x86_64/PageFault.hpp>
#include <Arch/x86_64/Registers.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>

static const char scancode_to_ascii[] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0,
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0,
	'*',
	0,
	' '
};

static const char* exception_names[] = {
	"Divide Error",
	"Debug Exception",
	"NMI Interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode (Undefined Opcode)",
	"Device Not Available (No Math Coprocessor)",
	"Double Fault",
	"Coprocessor Segment Overrun (reserved)",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection",
	"Page Fault",
	"Intel reserved. Do not use.",
	"x87 FPU Floating-Point Error (Math Fault)",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception",
	"Control Protection Exception"
};

extern "C" void isr_handler(Registers* registers) {
	// TODO: Consolidate Page Fault, Exceptions and Panic into one register & stack dumper
	if (registers->int_no == 14)
		return handle_page_fault(registers);

	if (registers->int_no < 32) {
		Logger::log_critical("Exception %d", registers->int_no);

		if (registers->int_no < sizeof(exception_names)) {
			Logger::kprintf(" - %s\n", exception_names[registers->int_no]);
		}

		Logger::log_critical("RIP=%X\tRSP=%X\n", registers->frame.rip, registers->frame.rsp);
		Logger::log_critical("RAX=%X\tRBX=%X\n", registers->rax, registers->rbx);
		Logger::log_critical("RCX=%X\tRDX=%X\n", registers->rcx, registers->rdx);

		for (;;) {
			asm volatile("cli; hlt");
		}
	}

	Logger::log_debug("Received an interrupt :^)\n");
	Logger::log_debug("\tInterrupt no. %d\n", registers->int_no);

	// TODO: Handle other interrupts
}
