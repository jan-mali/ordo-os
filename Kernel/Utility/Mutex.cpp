#include <Core/Multitasking/Scheduler.hpp>
#include <Utility/Mutex.hpp>
#include <Utility/Spinlock.hpp>

void Mutex::lock() {
	internal_lock.acquire();

	while (locked) {
		// waiters.sleep() assumes we have the lock locked
		// it only returns control after we have woken up
		waiters.sleep(internal_lock);
	}

	locked = true;
	owner = Multitasking::scheduler->current_thread();

	internal_lock.release();
}

void Mutex::unlock() {
	internal_lock.acquire();

	if (owner != Multitasking::scheduler->current_thread())
		panic("Mutex unlock by non-owner");

	locked = false;
	owner = nullptr;

	waiters.wake_one();

	internal_lock.release();
}
