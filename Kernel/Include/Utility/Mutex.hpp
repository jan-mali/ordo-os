#pragma once

#include <Core/Multitasking/Thread.hpp>
#include <Utility/Spinlock.hpp>
#include <Utility/WaitQueue.hpp>

class Mutex {
public:
	void lock();
	void unlock();

private:
	Spinlock internal_lock;
	bool locked;
	Multitasking::Thread* owner;
	WaitQueue waiters;
};
