#include <Arch/x86_64/IO.hpp>
#include <Arch/x86_64/PageFault.hpp>
#include <Arch/x86_64/Registers.hpp>
#include <Core/ACPI/Interrupts.hpp>
#include <Core/MM/KernelVMM.hpp>
#include <Core/MM/PMM.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/TextConsole.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>

static bool shift = false;

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

static const char scancode_to_ascii_shift[] = {
	0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	0,
	'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
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

extern "C" uint64_t isr_handler(Registers* registers) {
	// TODO: Consolidate Page Fault, Exceptions and Panic into one register & stack dumper
	if (registers->int_no == 14) {
		handle_page_fault(registers);
		return 0;
	}

	if (registers->int_no < 32) {
		Logger::reset_indent();
		Logger::log_critical("Exception %d", registers->int_no);

		if (registers->int_no < sizeof(exception_names)) {
			Logger::kprintf(" - %s\n", exception_names[registers->int_no]);
		}

		Logger::log_critical("Error Code: %X\n", registers->err_code);

		Logger::log_critical("RIP=%X\tRSP=%X\n", registers->frame.rip, registers->frame.rsp);
		Logger::log_critical("RAX=%X\tRBX=%X\n", registers->rax, registers->rbx);
		Logger::log_critical("RCX=%X\tRDX=%X\n", registers->rcx, registers->rdx);

		for (;;) {
			asm volatile("cli; hlt");
		}
	}

	if (registers->int_no == 32) {
		// LAPIC Timer
		ACPI::Interrupts::interrupt_manager->lapic().eoi();
		return Multitasking::scheduler->yield_from_interrupt(registers);
	}

	if (registers->int_no == 127) {
		// Scheduler yield
		return Multitasking::scheduler->yield_from_interrupt(registers);
	}

	if (registers->int_no == 33) {
		// PS/2 Keyboard
		uint8_t scancode = IO::inb(0x60);

		if ((scancode & ~0x80) == 42 || (scancode & ~0x80) == 54) {
			// Shift Key
			shift = !(scancode & 0x80);
		} else if (scancode < sizeof(scancode_to_ascii)) {
			text_console.set_color(0x00FFD700, 0);
			text_console.putc((shift ? scancode_to_ascii_shift : scancode_to_ascii)[scancode]);
			text_console.set_color(0xffffff, 0);
		} else {
			if (scancode == 59) { // F1
				Logger::kprintf("\n");
				Logger::kprintf("   ___          _          ___  ____  \n");
				Logger::kprintf("  / _ \\ _ __ __| | ___    / _ \\/ ___| \n");
				Logger::kprintf(" | | | | '__/ _` |/ _ \\  | | | \\___ \\ \n");
				Logger::kprintf(" | |_| | | | (_| | (_) | | |_| |___) |\n");
				Logger::kprintf("  \\___/|_|  \\__,_|\\___/   \\___/|____/ \n");
				Logger::kprintf("\n\t\tOrdo OS by Gameefan\n");
				Logger::kprintf("\n\n");
			}
			if (scancode == 60) { // F2
				Logger::log_debug("Memory statistics:\n");
				Logger::log_debug("PMM:\t Total: %d pages\n", pmm.get_total_pages());
				Logger::log_debug("\t\t Free: %d pages\n", pmm.get_free_pages());
				Logger::log_debug("VMM:\t Total: %d pages\n", kernel_vmm.get_total_pages());
				Logger::log_debug("\t\t Free: %d pages\n", kernel_vmm.get_free_pages());
			}
			if (scancode == 61) { // F3
				Multitasking::scheduler->dump();
			}
		}

		ACPI::Interrupts::interrupt_manager->lapic().eoi();
		return 0;
	}

	// TODO: Handle other interrupts

	Logger::log_debug("TODO: Handle other interrupts\n");
	Logger::log_debug("Received an interrupt :^)\n");
	Logger::log_debug("\tInterrupt no. %d\n", registers->int_no);

	return 0;
}
