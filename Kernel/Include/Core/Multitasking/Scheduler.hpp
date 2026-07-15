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

	inline Thread* current_thread() {
		return current;
	}

private:
	Vector<Thread*> threads;
	Thread* current;
};

void init();

extern Scheduler* scheduler;

}
