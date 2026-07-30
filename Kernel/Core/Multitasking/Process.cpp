#include <Core/ELFLoader.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Multitasking/Process.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>

namespace Multitasking {

static uint64_t max_process_id = 0;

Result<Process*> create_process(const char* path) {
	Process* process = new Process();

	process->address_space = Paging::AddressSpace::create_kernel_address_space();
	process->id = ++max_process_id;
	process->is_user_logging = false;

	ELFImage elf;

	if (!load_elf(path, process->address_space, elf)) {
		delete process;
		return nullptr;
	}

	process->elf_entry = elf.entry;

	Result<Thread*> main_thread_result = create_user_thread(process, path);
	if (!main_thread_result) {
		delete process;
		return main_thread_result.error();
	}

	process->main_thread = main_thread_result.value();

	scheduler->add_thread(process->main_thread);

	return process;
}

}
