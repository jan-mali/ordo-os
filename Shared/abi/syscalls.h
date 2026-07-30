#pragma once

#include <stdint.h>

#ifdef __cplusplus
enum class Syscall : uint64_t {
#else
typedef uint64_t Syscall;

enum {
#endif

	SYS_EXIT = 1,
	SYS_WRITE = 2

#ifdef __cplusplus
};
#else
};
#endif
