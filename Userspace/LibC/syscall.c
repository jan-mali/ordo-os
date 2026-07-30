#include <abi/syscalls.h>
#include <stdint.h>
#include <syscall.h>

int64_t syscall(uint64_t number, uint64_t a, uint64_t b, uint64_t c) {
	int64_t ret;

	__asm__ __volatile__(
		"int $0x80"
		: "=a"(ret)
		: "a"(number), // rax
		  "D"(a),	   // rdi
		  "S"(b),	   // rsi
		  "d"(c)	   // rdx
		: "memory"
	);

	return ret;
}

void exit(int status) {
	syscall(SYS_EXIT, status, 0, 0);

	while (1) {
		__asm__ __volatile__("1: jmp 1b");
	}
}

int64_t write(int fd, const void* buffer, uint64_t count) {
	return syscall(SYS_WRITE, fd, (uint64_t)buffer, count);
}
