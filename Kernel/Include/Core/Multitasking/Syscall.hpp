#pragma once

#include <Arch/x86_64/Registers.hpp>
#include <stdint.h>

namespace Multitasking {

void handle_syscall(Registers* regs);

void sys$exit(int code);
int sys$write(int fd, const void* user_buffer, uint64_t count);

}
