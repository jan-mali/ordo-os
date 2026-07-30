#pragma once

#include <Core/MM/Paging.hpp>
#include <Core/Result.hpp>

namespace Multitasking {

struct Thread;

struct Process {
	Paging::AddressSpace address_space;
	uint64_t id;
	Thread* main_thread;

	uint64_t elf_entry;

	bool is_user_logging;
};

Result<Process*> create_process(const char* path);

}
