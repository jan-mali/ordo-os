#pragma once

#include <Utility/RawSpinlock.hpp>
#include <Utility/WaitQueue.hpp>

class Semaphore {
public:
	explicit Semaphore(int initial)
		: count(initial) {}

	void wait();
	void signal();

private:
	RawSpinlock internal_lock;
	uint64_t count;
	WaitQueue waiters;
};
