#include <syscall.h>

int main() {
	const char* msg = "Hello from userspace!\n";

	write(1, msg, 23);

	return 42;
}
