#pragma once

#include <stdint.h>

#ifdef __cplusplus
enum class Errno : uint64_t {
#else
typedef uint64_t Errno;

enum {
#endif

	EPERM = 1,	// Operation not permitted
	ENOENT = 2, // No such file or directory
	ESRCH = 3,	// No such process

	EIO = 5, // I/O error

	EBADF = 9,	 // Bad file descriptor
	ECHILD = 10, // No child processes

	ENOMEM = 12, // Out of memory
	EACCES = 13, // Permission denied
	EFAULT = 14, // Bad user pointer

	EBUSY = 16,	 // Device/resource busy
	EEXIST = 17, // File exists

	ENODEV = 19,  // No such device
	ENOTDIR = 20, // Not a directory
	EISDIR = 21,  // Is a directory
	EINVAL = 22,  // Invalid argument

	ENOSPC = 28, // No space left on device

	ERANGE = 34, // Result too large

	ENOSYS = 38, // Syscall not implemented

#ifdef __cplusplus
};
#else
};
#endif
