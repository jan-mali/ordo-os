#include <Core/BufferTools.hpp>
#include <Core/Multitasking/Process.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Syscall.hpp>
#include <Core/Result.hpp>
#include <Logger/Logger.hpp>

namespace Multitasking {

void handle_syscall(Registers* regs) {
	uint64_t number = regs->rax;
	uint64_t a = regs->rdi;
	uint64_t b = regs->rsi;
	uint64_t c = regs->rdx;

	Result<uint64_t> result = Error {
		.domain = ErrorDomain::Generic,
		.code = ErrorCode::InvalidArgument
	};

	switch (number) {
	case 1:
		result = sys$exit(a);
		break;
	case 2:
		result = sys$write(a, reinterpret_cast<const char*>(b), c);
		break;
	default:
		break;
	}

	if (!result.has_value()) {
		regs->rax = -1;
		return;
	}

	regs->rax = result.value();
}

Result<uint64_t> sys$exit(int code) {
	// TODO: Handle multiple threads
	scheduler->current_thread()->state = ThreadState::Dead;

	Logger::log_info("sys$exit: Exiting with code %d\n", code);

	while (true) {
		scheduler->yield();
	}
}

Result<uint64_t> sys$write(int fd, const char* user_buffer, uint64_t count) {
	Process* process = scheduler->current_thread()->process;

	if (fd != 1 && fd != 2) {
		return Error {
			.domain = ErrorDomain::Process,
			.code = ErrorCode::InvalidHandle
		};
	}

	char text_buffer[1024] = "0";

	// TODO: Use this
	(void)count;

	TRY(BufferTools::strncpy_from_user(text_buffer, user_buffer, 1024));

	process->is_user_logging = true;

	Logger::log_info("%s", text_buffer);

	process->is_user_logging = false;

	return 0;
}

}
