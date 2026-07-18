#pragma once

#include <Utility/LockGuard.hpp>
#include <Utility/RawSpinlock.hpp>

class IRQSpinlock {
public:
	IRQSpinlock() = default;

	void lock();
	void unlock();

private:
	RawSpinlock raw;
	uint64_t flags;
};

using IRQSpinlockGuard = LockGuard<IRQSpinlock>;
