#include <Arch/x86_64/Interrupts.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>
#include <Utility/IRQSpinlock.hpp>

namespace Multitasking {

void idle_task(void*);

Scheduler* scheduler;
Thread* idle_task_thread;

IRQSpinlock threads_lock;

void Scheduler::add_thread(Thread* thread) {
	IRQSpinlockGuard guard(threads_lock);

	threads.push_back(thread);
}

static size_t thread_index = 0;

void Scheduler::yield() {
	asm volatile("int $0x7F" ::: "memory");
}

uint64_t Scheduler::yield_from_interrupt(Registers* regs) {
	Thread* old = current;

	if (preemption_disable_count > 0) {
		// FIXME: SMP: this only protects this CPU.
		// A different CPU may still run another thread. So this does not make an operation magically thread-safe

		if (old->state != ThreadState::Running) {
			panic("Preemption is disabled but thread %d cannot be scheduled as it is %s.\n", old->id, thread_state_to_string(old->state));
		}
		return 0;
	}

	// NOTE: This is lock disables preemption so it needs to be run only after the preemption check
	IRQSpinlockGuard guard(threads_lock);

	// Logger::log_debug("Saving interrupt rsp %X for thread %d\n", regs, current->id);

	// Save the interrupted thread's stack
	old->interrupt_rsp = reinterpret_cast<uint64_t>(regs);

	Thread* next = nullptr;
	size_t next_index = thread_index;

	for (size_t i = 1; i <= threads.size(); i++) {
		size_t index = (thread_index + i) % threads.size();

		Thread* candidate = threads[index];

		if (candidate->state == ThreadState::Ready) {
			next = candidate;
			next_index = index;
			break;
		}
	}

	if (!next) {
		if (old->state == ThreadState::Running) {
			return reinterpret_cast<uint64_t>(regs);
		}
		next = idle_task_thread;
	}

	if (old->state == ThreadState::Running) {
		// Don't change the state if the thread is dead, blocked, sleeping, etc.
		old->state = ThreadState::Ready;
	}

	next->state = ThreadState::Running;

	current = next;
	thread_index = next_index;

	// Logger::log_debug("About to schedule thread %d, rsp=%X\n", current->id, current->interrupt_rsp);

	return next->interrupt_rsp;
}

void Scheduler::start() {
	if (threads.size() > 0) {
		current = threads[0];
	} else {
		current = idle_task_thread;
	}

	current->state = ThreadState::Running;
	thread_index = 0;
	enable_preemption();

	Logger::log_info("Scheduler: About to start scheduling.\n");

	// IRQs are enabled by restoring RFLAGS from the interrupt frame, so we don't need to enable them here.
	// In fact it would lead to a race condition since the timer could fire before we do schedule the first thread.
	restore_interrupt_context(current->interrupt_rsp);
}

void Scheduler::block() {
	current->state = ThreadState::Blocked;
}

void Scheduler::wake(Thread* thread) {
	// FIXME: What if the current thread was destroyed before it is meant to wake up?
	// Then this is a use-after-free
	// So maybe somehow guarantee that a blocked thread cannot possibly be destroyed?

	thread->state = ThreadState::Ready;
}

void Scheduler::dump() {
	IRQSpinlockGuard guard(threads_lock);

	Logger::log_debug("Scheduler dump:\n");
	Logger::log_debug("\t(idle) %d: %s %s\n", idle_task_thread->id, idle_task_thread->name, thread_state_to_string(idle_task_thread->state));
	for (size_t i = 0; i < threads.size(); i++) {
		Thread* thread = threads[i];
		Logger::log_debug("\t%d: %s %s\n", thread->id, thread->name, thread_state_to_string(thread->state));
	}
}

void init() {
	scheduler = new Scheduler();

	// Create the idle task
	idle_task_thread = Multitasking::create_thread(idle_task, nullptr, "idle_task");

	Logger::log_info("Multitasking is initialized.\n");
}

void idle_task(void*) {
	while (true) {
		asm volatile("hlt");
	}
}

}
