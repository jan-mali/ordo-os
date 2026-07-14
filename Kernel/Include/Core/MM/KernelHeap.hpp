#pragma once

#include <cstdint>

struct HeapBlock {
	uint64_t size;
	bool free;

	HeapBlock* prev;
	HeapBlock* next;
};

class KernelHeap {
public:
	void init();

	void* alloc(uint64_t size);
	void free(void* ptr);

private:
	HeapBlock* head;
	bool initialized;

	void expand_heap(uint64_t size);
};

extern KernelHeap kernel_heap;
