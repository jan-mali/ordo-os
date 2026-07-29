#include <stdint.h>
#include <syscall.h>

uint64_t syscall(uint64_t number, uint64_t a, uint64_t b, uint64_t c) {
	uint64_t ret;

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
