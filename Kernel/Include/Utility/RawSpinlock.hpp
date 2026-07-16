#pragma once

#include <cstdint>

class RawSpinlock {
public:
	void acquire() {
		while (__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE))
			asm volatile("pause");
	}

	void release() {
		__atomic_clear(&locked, __ATOMIC_RELEASE);
	}

private:
	volatile uint8_t locked = 0;
};
