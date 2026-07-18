#pragma once

#include <Utility/LockGuard.hpp>
#include <Utility/RawSpinlock.hpp>

class Spinlock {
public:
	Spinlock() = default;

	void lock();
	void unlock();

private:
	RawSpinlock raw;
};

using SpinlockGuard = LockGuard<Spinlock>;
