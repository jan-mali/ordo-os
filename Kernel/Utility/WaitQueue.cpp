#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Utility/Spinlock.hpp>
#include <Utility/Vector.hpp>
#include <Utility/WaitQueue.hpp>

void WaitQueue::sleep(Spinlock& external_lock) {
	Multitasking::Thread* current = Multitasking::scheduler->current_thread();

	// Assume the extern_lock is locked

	current->state = Multitasking::ThreadState::Blocked;
	waiters.push_back(current);

	external_lock.unlock();

	// If a preemption happens here we are already on the waiters list
	// so we get resumed if wake_one() is called

	Multitasking::scheduler->yield();

	// The only point at which execution comes back here (yield has returned & this function will return now)
	// is when we have woken up
}

void WaitQueue::wake_one() {
	// This function assumes that an external lock is already being held

	if (waiters.size() > 0) {
		Multitasking::Thread* waking_thread = waiters.remove_front();
		Multitasking::scheduler->wake(waking_thread);
	}
}

void WaitQueue::wake_all() {
	// This function assumes that an external lock is already being held

	while (!waiters.empty()) {
		Multitasking::scheduler->wake(waiters.remove_front());
	}
}
