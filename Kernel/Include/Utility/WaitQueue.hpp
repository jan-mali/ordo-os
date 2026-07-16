#pragma once

#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Utility/RawSpinlock.hpp>
#include <Utility/Vector.hpp>

class WaitQueue {
public:
	void sleep(RawSpinlock& external_lock);
	void wake_one();
	void wake_all();

private:
	Vector<Multitasking::Thread*> waiters;
};
