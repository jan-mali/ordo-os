#include <Core/MM/KernelVMM.hpp>
#include <Core/MM/PMM.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Multitasking/Process.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Core/Panic.hpp>
#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Utility/String.hpp>

namespace Multitasking {

constexpr uint32_t STACK_PAGES = 4;
static uint64_t max_thread_id = 0;

void thread_entry();

Result<Thread*> create_kernel_thread(void (*entry)(void*), void* argument, const char* name) {
	Thread* thread = new Thread();
	uint8_t* stack = reinterpret_cast<uint8_t*>(kernel_vmm.allocate_pages(STACK_PAGES, true));
	thread->id = ++max_thread_id;

	if (name) {
		size_t name_length = strlen(name);
		if (name_length > 0) {
			memcpy(thread->name, name, name_length > 32 ? 32 : name_length);
			thread->name[31] = 0;
		} else {
			memcpy(thread->name, "unnamed", sizeof("unnamed"));
		}
	} else {
		memcpy(thread->name, "unnamed", sizeof("unnamed"));
	}

	thread->stack_base = stack;
	thread->stack_top = stack + STACK_PAGES * Paging::PAGE_SIZE;
	thread->stack_size = STACK_PAGES * Paging::PAGE_SIZE;

	thread->user_stack_base = 0;
	thread->user_stack_top = 0;
	thread->user_stack_size = 0;

	thread->process = nullptr;

	thread->state = Multitasking::ThreadState::Ready;

	uint64_t* rsp = reinterpret_cast<uint64_t*>(thread->stack_top);
	*(--rsp) = 0x0; // Invalid address for stack unwinding

	// iretq frame
	*(--rsp) = 0x10;										  // SS
	*(--rsp) = reinterpret_cast<uint64_t>(thread->stack_top); // RSP
	*(--rsp) = 0x202;										  // RFLAGS (IF enabled)
	*(--rsp) = 0x08;										  // CS
	*(--rsp) = reinterpret_cast<uint64_t>(&thread_entry);	  // RIP

	// error code + interrupt number
	*(--rsp) = 0; // err_code
	*(--rsp) = 0; // int_no

	// registers pushed by the ISR
	*(--rsp) = 0; // r15
	*(--rsp) = 0; // r14
	*(--rsp) = 0; // r13
	*(--rsp) = 0; // r12
	*(--rsp) = 0; // r11
	*(--rsp) = 0; // r10
	*(--rsp) = 0; // r9
	*(--rsp) = 0; // r8

	*(--rsp) = 0; // rbp
	*(--rsp) = 0; // rdi
	*(--rsp) = 0; // rsi
	*(--rsp) = 0; // rdx
	*(--rsp) = 0; // rcx
	*(--rsp) = 0; // rbx
	*(--rsp) = 0; // rax

	thread->interrupt_rsp = reinterpret_cast<uint64_t>(rsp);

	thread->entry = entry;
	thread->argument = argument;

	return thread;
}

Result<Thread*> create_user_thread(Process* process, const char* name) {
	Thread* thread = new Thread();

	thread->id = ++max_thread_id;
	thread->process = process;

	if (name) {
		size_t name_length = strlen(name);
		if (name_length > 0) {
			memcpy(thread->name, name, name_length > 32 ? 32 : name_length);
			thread->name[31] = 0;
		} else {
			memcpy(thread->name, "unnamed", sizeof("unnamed"));
		}
	} else {
		memcpy(thread->name, "unnamed", sizeof("unnamed"));
	}

	// Allocate kernel stack
	uint8_t* stack = reinterpret_cast<uint8_t*>(kernel_vmm.allocate_pages(STACK_PAGES, true));
	thread->stack_base = stack;
	thread->stack_top = stack + STACK_PAGES * Paging::PAGE_SIZE;
	thread->stack_size = STACK_PAGES * Paging::PAGE_SIZE;

	// User stack
	constexpr uint64_t USER_STACK_TOP = 0x00007ffffff000;
	constexpr uint64_t USER_STACK_PAGES = 4;
	constexpr uint64_t USER_STACK_SIZE = USER_STACK_PAGES * Paging::PAGE_SIZE;
	constexpr uint64_t USER_STACK_BASE = USER_STACK_TOP - USER_STACK_SIZE;

	// FIXME: This will break for multiple threads, the stacks need to be different

	uint8_t* user_stack = reinterpret_cast<uint8_t*>(pmm.alloc(USER_STACK_PAGES));

	for (uint64_t addr = USER_STACK_BASE; addr < USER_STACK_TOP; addr += Paging::PAGE_SIZE) {
		auto page = pmm.alloc(1);

		process->address_space.map(addr, reinterpret_cast<uint64_t>(page), static_cast<uint64_t>(Paging::PageFlags::User) | static_cast<uint64_t>(Paging::PageFlags::Writable));
	}

	thread->user_stack_base = reinterpret_cast<void*>(USER_STACK_BASE);
	thread->user_stack_top = reinterpret_cast<void*>(USER_STACK_TOP);
	thread->user_stack_size = USER_STACK_SIZE;

	uint64_t* rsp = reinterpret_cast<uint64_t*>(thread->stack_top);

	// iretq frame

	*(--rsp) = 0x20 | 3; // SS (user data)
	*(--rsp) = USER_STACK_TOP;
	*(--rsp) = 0x202;	 // RFLAGS, IF enabled
	*(--rsp) = 0x18 | 3; // CS (user code)
	*(--rsp) = process->elf_entry;

	// fake interrupt frame

	*(--rsp) = 0; // err code
	*(--rsp) = 0; // int number

	// registers

	*(--rsp) = 0; // r15
	*(--rsp) = 0; // r14
	*(--rsp) = 0; // r13
	*(--rsp) = 0; // r12
	*(--rsp) = 0; // r11
	*(--rsp) = 0; // r10
	*(--rsp) = 0; // r9
	*(--rsp) = 0; // r8

	*(--rsp) = 0; // rbp
	*(--rsp) = 0; // rdi
	*(--rsp) = 0; // rsi
	*(--rsp) = 0; // rdx
	*(--rsp) = 0; // rcx
	*(--rsp) = 0; // rbx
	*(--rsp) = 0; // rax

	thread->interrupt_rsp = reinterpret_cast<uint64_t>(rsp);

	thread->state = ThreadState::Ready;

	return thread;
}

void thread_entry() {
	uint64_t rsp;
	asm volatile("mov %%rsp, %0" : "=r"(rsp));

	Logger::log_debug("thread_entry: Thread %d stack=%016llX\n", scheduler->current_thread()->id, rsp);
	Logger::log_debug("thread_entry: About to call the thread entrypoint at %p\n", scheduler->current_thread()->entry);

	// Call the thread entry function
	scheduler->current_thread()->entry(scheduler->current_thread()->argument);

	// If we returned, the thread should die
	scheduler->current_thread()->state = ThreadState::Dead;

	Logger::log_debug("thread_entry: Thread %d returned control\n", scheduler->current_thread()->id);

	// TODO: Kill these threads at some point
	while (true) {
		scheduler->yield();
	}
}

}
