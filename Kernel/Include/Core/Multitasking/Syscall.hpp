#pragma once

#include <Arch/x86_64/Registers.hpp>
#include <Core/Result.hpp>
#include <stdint.h>

namespace Multitasking {

void handle_syscall(Registers* regs);

Result<uint64_t> sys$exit(int code);
Result<uint64_t> sys$write(int fd, const char* user_buffer, uint64_t count);

}
