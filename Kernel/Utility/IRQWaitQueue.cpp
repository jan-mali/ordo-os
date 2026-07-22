#include <Arch/x86_64/Interrupts.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Utility/IRQWaitQueue.hpp>
#include <Utility/RawSpinlock.hpp>
#include <Utility/Vector.hpp>

void IRQWaitQueue::sleep(RawSpinlock& external_lock, uint64_t* flags) {
	Multitasking::Thread* current = Multitasking::scheduler->current_thread();

	// Assume the extern_lock is locked & IRQs are disabled

	current->state = Multitasking::ThreadState::Blocked;
	waiters.push_back(current);

	external_lock.release();

	Interrupts::restore(*flags);

	// If a preemption happens here we are already on the waiters list
	// so we get resumed if wake_one() is called

	Multitasking::scheduler->yield();

	// The only point at which execution comes back here (yield has returned & this function will return now)
	// is when we have woken up

	*flags = Interrupts::store_and_disable();

	external_lock.acquire();
}

void IRQWaitQueue::wake_one() {
	// This function assumes that an external lock is already being held & IRQs are disabled

	if (waiters.size() > 0) {
		Multitasking::Thread* waking_thread = waiters.remove_front();
		Multitasking::scheduler->wake(waking_thread);
	}
}

void IRQWaitQueue::wake_all() {
	// This function assumes that an external lock is already being held & IRQs are disabled

	while (!waiters.empty()) {
		Multitasking::scheduler->wake(waiters.remove_front());
	}
}
