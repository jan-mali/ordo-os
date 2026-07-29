#include <Core/Multitasking/Process.hpp>
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
		sys$exit(a);
		break;
	case 2:
		regs->rax = sys$write(a, reinterpret_cast<const void*>(b), c);
		break;
	default:
		break;
	}
}

static bool is_user_pointer(uint64_t ptr) {
	return ptr < 0x0000800000000000;
}

void sys$exit(int code) {
	// TODO: Handle multiple threads
	scheduler->current_thread()->state = ThreadState::Dead;

	Logger::log_info("sys$exit: Exiting with code %d\n", code);

	while (true) {
		scheduler->yield();
	}
}

int sys$write(int fd, const void* user_buffer, uint64_t count) {
	Process* process = scheduler->current_thread()->process;

	if (fd != 1 && fd != 2)
		return -1;

	if (!is_user_pointer(reinterpret_cast<uint64_t>(user_buffer)))
		return -1;

	const char* data = reinterpret_cast<const char*>(user_buffer);

	process->is_user_logging = true;

	Logger::log_info("%s", data);

	process->is_user_logging = false;

	return 0;
}

}
