#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Syscall.hpp>
#include <Logger/Logger.hpp>

namespace Multitasking {

void handle_syscall(Registers* regs) {
	uint64_t number = regs->rax;
	uint64_t a = regs->rdi;
	uint64_t b = regs->rsi;
	uint64_t c = regs->rdx;

	switch (number) {
	case 1:
		return sys$exit(a);
	default:
		break;
	}
}

void sys$exit(int code) {
	// TODO: Handle multiple threads
	scheduler->current_thread()->state = ThreadState::Dead;

	Logger::log_info("sys$exit: Exiting with code %d\n", code);

	while (true) {
		scheduler->yield();
	}
}

}
