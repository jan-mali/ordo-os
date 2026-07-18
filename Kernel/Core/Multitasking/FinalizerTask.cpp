#include <Core/MM/KernelVMM.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Multitasking/FinalizerTask.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Logger/Logger.hpp>
#include <Utility/IRQSpinlock.hpp>

namespace Multitasking {

void FinalizerTask::start() {
	scheduler->add_thread(create_thread(finalizer_task, nullptr, "finalizer_task"));
}

void FinalizerTask::finalizer_task(void*) {
	Logger::log_info("finalizer_task started.\n");

	while (true) {
		{
			IRQSpinlockGuard guard(scheduler->threads_lock);
			for (int i = scheduler->threads.size() - 1; i >= 0; --i) {
				Thread* thread = scheduler->threads[i];

				if (thread->state != ThreadState::Dead) {
					continue;
				}

				Logger::log_info("Finalizing thread %d (%s)\n", thread->id, thread->name);

				// Clean up the thread's resources

				thread->argument = nullptr;
				thread->entry = nullptr;
				thread->interrupt_rsp = 0;

				thread->process = nullptr;

				kernel_vmm.free_pages(thread->stack_base, thread->stack_size / Paging::PAGE_SIZE);
				thread->stack_base = nullptr;
				thread->stack_size = 0;
				thread->stack_top = 0;

				scheduler->threads.erase(i);

				delete thread;
			}
		}

		scheduler->yield();
	}
}

}
