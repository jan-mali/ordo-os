#include <Core/Multitasking/Scheduler.hpp>
#include <Utility/Mutex.hpp>
#include <Utility/Spinlock.hpp>

void Mutex::lock() {
	internal_lock.lock();

	while (locked) {
		// waiters.sleep() assumes we have the lock locked
		// it also unlocks the lock but only returns control after we have woken up
		waiters.sleep(internal_lock);
		internal_lock.lock();
	}

	locked = true;
	owner = Multitasking::scheduler->current_thread();

	internal_lock.unlock();
}

void Mutex::unlock() {
	SpinlockGuard guard(internal_lock);

	locked = false;
	owner = nullptr;

	waiters.wake_one();
}
