#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Logger/Logger.hpp>

namespace Multitasking {

Scheduler* scheduler;

void Scheduler::add_thread(Thread* thread) {
	threads.push_back(thread);
}

static size_t thread_index = 0;

void Scheduler::yield() {
	Thread* old = current;

	Thread* next = nullptr;
	size_t next_index = thread_index;

	for (size_t i = 1; i <= threads.size(); i++) {
		size_t idx = (thread_index + i) % threads.size();
		Thread* candidate = threads[idx];

		if (candidate->state == ThreadState::Ready) {
			next = candidate;
			next_index = idx;
			break;
		}
	}

	if (!next) {
		Logger::log_error("Scheduler: Failed to find next thread to schedule\n");
		return;
	}

	thread_index = next_index;

	Logger::log_debug("Scheduler: About to schedule thread %d\n", next->id);

	if (old->state == ThreadState::Running) {
		// If the thread's state changed we don't want to reschedule it (it is dead, blocking, sleeping, etc)
		old->state = ThreadState::Ready;
	}
	next->state = ThreadState::Running;

	current = next;

	switch_context(
		&old->context,
		&next->context
	);
}

void Scheduler::start() {
	CPUContext boot_context;
	current = threads[0];

	current->state = ThreadState::Running;

	switch_context(
		&boot_context,
		&current->context
	);
}

void init() {
	scheduler = new Scheduler();

	Logger::log_info("Multitasking is initialized.\n");
}

}
