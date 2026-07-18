#include <Arch/x86_64/Interrupts.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Utility/IRQSpinlock.hpp>

void IRQSpinlock::lock() {
	// This order is fine because it is impossible for this spinlock to be locked with preemption enabled
	// since if another thread has it locked, then we can't be executing since that thread couldn't have
	// been preempted in the first place.

	flags = Interrupts::store_and_disable();
	if (Multitasking::scheduler) {
		Multitasking::scheduler->disable_preemption();
	}
	raw.acquire();
}

void IRQSpinlock::unlock() {
	raw.release();
	if (Multitasking::scheduler) {
		Multitasking::scheduler->enable_preemption();
	}
	Interrupts::restore(flags);
}
