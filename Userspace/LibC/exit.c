#include <exit.h>
#include <syscall.h>

void exit(int status) {
	syscall(1, status, 0, 0);

	while (1) {
		__asm__ __volatile__("1: jmp 1b");
	}
}
