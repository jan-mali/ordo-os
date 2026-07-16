#pragma once

#include <Core/Multitasking/Thread.hpp>
#include <Utility/RawSpinlock.hpp>
#include <Utility/WaitQueue.hpp>

class Mutex {
public:
	void lock();
	void unlock();

private:
	RawSpinlock internal_lock;
	bool locked;
	Multitasking::Thread* owner;
	WaitQueue waiters;
};
