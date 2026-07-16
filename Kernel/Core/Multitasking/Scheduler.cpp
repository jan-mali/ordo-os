#include <Arch/x86_64/Interrupts.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>

namespace Multitasking {

void idle_task(void*);

Scheduler* scheduler;
Thread* idle_task_thread;

void Scheduler::add_thread(Thread* thread) {
	threads.push_back(thread);
}

static size_t thread_index = 0;

void Scheduler::yield() {
	asm volatile("int $0x7F" ::: "memory");
}

uint64_t Scheduler::yield_from_interrupt(Registers* regs) {
	Thread* old = current;

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

	Logger::log_info("Scheduler: About to start scheduling.\n");

	// IRQs are enabled by restoring RFLAGS from the interrupt frame, so we don't need to enable them here.
	// In fact it would lead to a race condition since the timer could fire before we do schedule the first thread.
	restore_interrupt_context(current->interrupt_rsp);
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
