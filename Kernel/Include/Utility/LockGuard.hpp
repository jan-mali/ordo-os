#pragma once

template<typename Lock>
class LockGuard {
public:
	explicit LockGuard(Lock& lock)
		: lock(lock) {
		this->lock.lock();
	}

	~LockGuard() {
		lock.unlock();
	}

	// Prevent copying
	LockGuard(const LockGuard&) = delete;
	LockGuard& operator=(const LockGuard&) = delete;

private:
	Lock& lock;
};
