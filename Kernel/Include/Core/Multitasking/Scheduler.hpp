#pragma once

#include <Arch/x86_64/Interrupts.hpp>
#include <Arch/x86_64/Registers.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>
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
		// This is a critical section
		uint64_t flags = Interrupts::store_and_disable();
		preemption_disable_count++;
		Interrupts::restore(flags);
	}

	void enable_preemption() {
		// This is a critical section
		uint64_t flags = Interrupts::store_and_disable();
		if (preemption_disable_count > 0) {
			preemption_disable_count--;
		}
		Interrupts::restore(flags);
	}

	inline Thread* current_thread() {
		return current;
	}

private:
	Vector<Thread*> threads;
	Thread* current;
	uint32_t preemption_disable_count;
};

void init();

extern Scheduler* scheduler;

}
