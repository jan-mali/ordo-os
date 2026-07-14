#include <Core/MM/KernelHeap.hpp>
#include <Core/MM/KernelVMM.hpp>
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>

KernelHeap kernel_heap;

void KernelHeap::init() {
	void* page = kernel_vmm.allocate_pages(16);

	head = reinterpret_cast<HeapBlock*>(page);

	head->size = 16 * 4096 - sizeof(HeapBlock);
	head->free = true;
	head->next = nullptr;
	head->prev = nullptr;

	initialized = true;

	Logger::log_info("KernelHeap: Initialized with 16 pages (64 KiB).\n");
}

void* KernelHeap::alloc(uint64_t size) {
	if (!initialized) {
		panic("KernelHeap: Not yet initialized.\n");
	}

	size = (size + 15) & ~15;

	HeapBlock* block = head;

	while (block) {
		if (block->free && block->size >= size) {
			block->free = false;

			return block + 1;
		}

		block = block->next;
	}

	expand_heap(size);

	return alloc(size);
}

void KernelHeap::free(void* ptr) {
	if (!initialized) {
		panic("KernelHeap: Not yet initialized.\n");
	}

	if (!ptr)
		return;

	HeapBlock* block = reinterpret_cast<HeapBlock*>(ptr) - 1;

	block->free = true;
}

void KernelHeap::expand_heap(uint64_t size) {
	uint64_t pages = (size + sizeof(HeapBlock) + 4095) / 4096;

	auto* block = reinterpret_cast<HeapBlock*>(
		kernel_vmm.allocate_pages(pages)
	);

	block->size = pages * 4096 - sizeof(HeapBlock);
	block->free = true;

	HeapBlock* current = head;

	while (current->next) {
		current = current->next;
	}

	current->next = block;
	block->prev = current;
}
