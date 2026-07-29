#pragma once

#include <Arch/x86_64/Registers.hpp>
#include <stdint.h>

namespace Multitasking {

void handle_syscall(Registers* regs);

void sys$exit(int code);

}
