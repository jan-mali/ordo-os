#pragma once

#include <Core/Multitasking/Thread.hpp>
#include <Utility/Vector.hpp>

namespace Multitasking {

extern "C" void switch_context(CPUContext* old, CPUContext* next);

class Scheduler {
public:
	void add_thread(Thread* thread);
	void yield();
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
