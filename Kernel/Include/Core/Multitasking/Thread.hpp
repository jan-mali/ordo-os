#pragma once

#include <cstddef>
#include <cstdint>

namespace Multitasking {

// Just the callee-saved registers since context switching will be a normal function call for now
struct CPUContext {
	uint64_t rsp;

	uint64_t rbx;
	uint64_t rbp;

	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
};

enum class ThreadState {
	Ready,
	Running,
	Dead
};

struct Thread {
	uint64_t id;

	CPUContext context;
	ThreadState state;

	void (*entry)(void*);
	void* arguemnt;

	void* stack_base;
	void* stack_top;
	size_t stack_size;
};

Thread* create_thread(void (*entry)(void*), void* argument);

}
