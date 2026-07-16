#pragma once

#include <cstddef>
#include <cstdint>

struct Process;

namespace Multitasking {

enum class ThreadState {
	Ready,
	Running,
	Sleeping,
	Blocked,
	Dead
};

inline const char* thread_state_to_string(ThreadState state) {
	switch (state) {
	case ThreadState::Ready:
		return "Ready";
	case ThreadState::Running:
		return "Running";
	case ThreadState::Sleeping:
		return "Sleeping";
	case ThreadState::Blocked:
		return "Blocked";
	case ThreadState::Dead:
		return "Dead";
	}

	return "Invalid";
}

struct Thread {
	uint64_t id;

	uint64_t interrupt_rsp;

	void (*entry)(void*);
	void* arguemnt;

	void* stack_base;
	void* stack_top;
	size_t stack_size;

	ThreadState state;

	char name[32];

	Process* process;
};

Thread* create_thread(void (*entry)(void*), void* argument, const char* name);

}
