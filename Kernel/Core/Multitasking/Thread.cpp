#include <Core/MM/KernelVMM.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>

namespace Multitasking {

constexpr uint32_t STACK_PAGES = 4;
static uint32_t max_thread_id = 0;

void thread_entry();

Thread* create_thread(void (*entry)(void*), void* argument) {
	Thread* thread = new Thread();
	uint8_t* stack = reinterpret_cast<uint8_t*>(kernel_vmm.allocate_pages(STACK_PAGES));
	thread->id = ++max_thread_id;

	thread->stack_base = stack;
	// FIXME: Use Paging::PAGE_SIZE
	thread->stack_top = stack + STACK_PAGES * 4096;
	thread->stack_size = STACK_PAGES * 4096;

	thread->state = Multitasking::ThreadState::Ready;

	uint64_t* rsp = reinterpret_cast<uint64_t*>(thread->stack_top);
	*(--rsp) = 0;										  // Invalid return address just in case
	*(--rsp) = reinterpret_cast<uint64_t>(&thread_entry); // Task start
	*(--rsp) = 0;										  // rbx
	*(--rsp) = 0;										  // rbp
	*(--rsp) = 0;										  // r12
	*(--rsp) = 0;										  // r13
	*(--rsp) = 0;										  // r14
	*(--rsp) = 0;										  // r15

	thread->context.rsp = reinterpret_cast<uint64_t>(rsp);

	thread->entry = entry;
	thread->arguemnt = argument;

	return thread;
}

void thread_entry() {
	// Call the thread entry function
	scheduler->current_thread()->entry(scheduler->current_thread()->arguemnt);

	// If we returned, the thread should die
	scheduler->current_thread()->state = ThreadState::Dead;

	Logger::log_debug("thread_entry: Thread %d returned control\n", scheduler->current_thread()->id);

	scheduler->yield();

	panic("Dead thread was scheduled.\n");
}

}
