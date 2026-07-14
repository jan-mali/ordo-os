#include <Arch/x86_64/PageFault.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>

static inline uint64_t read_cr2() {
	uint64_t value;
	asm volatile("mov %%cr2, %0" : "=r"(value));
	return value;
}

static inline uint64_t read_cr3() {
	uint64_t value;
	asm volatile("mov %%cr3, %0" : "=r"(value));
	return value;
}

void handle_page_fault(Registers* regs) {
	uint64_t cr2 = read_cr2();
	uint64_t cr3 = read_cr3();

	Logger::log_critical("Exception %d - Page Fault\n", regs->int_no);

	uint64_t err = regs->err_code;
	Logger::log_critical(
		"PF error: %s %s %s\n",
		err & 1 ? "protection" : "not-present",
		err & 2 ? "write" : "read",
		err & 4 ? "user" : "kernel"
	);

	Logger::log_critical("CR2  = 0x%X (fault address)\n", cr2);
	Logger::log_critical("CR3  = 0x%X (PML4 base)\n", cr3);

	Logger::log_critical("RIP=%X\tRSP=%X\n", regs->frame.rip, regs->frame.rsp);
	Logger::log_critical("RAX=%X\tRBX=%X\n", regs->rax, regs->rbx);
	Logger::log_critical("RCX=%X\tRDX=%X\n", regs->rcx, regs->rdx);

	// Right now we don't have any way of recovering
	panic("Page Fault in kernel code.\n");
}
