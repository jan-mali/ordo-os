#pragma once

#include <cstddef>
#include <cstdint>

namespace Multitasking {

enum class ThreadState {
	Ready,
	Running,
	Dead
};

struct Thread {
	uint64_t id;

	uint64_t interrupt_rsp;

	void (*entry)(void*);
	void* arguemnt;

	void* stack_base;
	void* stack_top;
	size_t stack_size;

	ThreadState state;
};

Thread* create_thread(void (*entry)(void*), void* argument);

}
