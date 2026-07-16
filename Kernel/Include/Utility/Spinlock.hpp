#pragma once

#include <Core/Multitasking/Scheduler.hpp>
#include <Utility/LockGuard.hpp>
#include <Utility/RawSpinlock.hpp>
#include <stdint.h>

class Spinlock {
public:
	Spinlock() = default;

	void lock() {
		// This order is fine because it is impossible for this spinlock to be locked with preemption enabled
		// since if another thread has it locked, then we can't be executing since that thread couldn't have
		// been preempted in the first place.

		if (Multitasking::scheduler) {
			Multitasking::scheduler->disable_preemption();
		}
		raw.acquire();
	}

	void unlock() {
		raw.release();
		if (Multitasking::scheduler) {
			Multitasking::scheduler->enable_preemption();
		}
	}

private:
	RawSpinlock raw;
};

using SpinlockGuard = LockGuard<Spinlock>;
