#include <Core/ELFLoader.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Multitasking/Process.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>

namespace Multitasking {

static uint64_t max_process_id = 0;

Process* create_process(const char* path) {
	Process* process = new Process();

	process->address_space = Paging::AddressSpace::create_kernel_address_space();
	process->id = ++max_process_id;

	ELFImage elf;

	if (!load_elf(path, process->address_space, elf)) {
		delete process;
		return nullptr;
	}

	process->elf_entry = elf.entry;
	process->main_thread = create_user_thread(process, path);

	scheduler->add_thread(process->main_thread);

	return process;
}

}
