#pragma once

#include <Arch/x86_64/Registers.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Utility/Vector.hpp>

namespace Multitasking {

extern "C" void restore_interrupt_context(uint64_t rsp);

class Scheduler {
public:
	void add_thread(Thread* thread);

	void yield();
	uint64_t yield_from_interrupt(Registers* regs);

	void start();

	void block();
	void wake(Thread* thread);

	void dump();

	void disable_preemption() {
		preemption_disabled = true;
	}

	void enable_preemption() {
		preemption_disabled = false;
	}

	inline Thread* current_thread() {
		return current;
	}

private:
	Vector<Thread*> threads;
	Thread* current;
	bool preemption_disabled;
};

void init();

extern Scheduler* scheduler;

}
