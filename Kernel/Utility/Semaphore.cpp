#include <Core/Multitasking/Scheduler.hpp>
#include <Utility/Semaphore.hpp>

void Semaphore::wait() {
	internal_lock.acquire();

	while (count == 0) {
		waiters.sleep(internal_lock);
	}

	count--;

	internal_lock.release();
}

void Semaphore::signal() {
	internal_lock.acquire();

	count++;

	waiters.wake_one();

	internal_lock.release();
}
