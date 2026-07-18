#pragma once

#include <Core/Bitmap.hpp>
#include <cstdint>
#include <limine.h>

constexpr uint64_t KERNEL_HEAP_START = 0xFFFF900000000000;
constexpr uint64_t KERNEL_HEAP_SIZE = 4LLU * 1024 * 1024 * 1024; // 4 GiB
constexpr uint64_t KERNEL_HEAP_END = KERNEL_HEAP_START + KERNEL_HEAP_SIZE;
constexpr uint64_t KERNEL_HEAP_PAGES = KERNEL_HEAP_SIZE / 4096;
constexpr uint64_t KERNEL_HEAP_BITMAP_PAGES = (KERNEL_HEAP_PAGES + 7) / 8 / 4096;

class KernelVMM {
public:
	void init();

	void* allocate_pages(uint64_t count, bool contiguous = false);
	void free_pages(const void* ptr, uint64_t count);

	uint64_t get_total_pages() const { return stats.pages_total; }
	uint64_t get_free_pages() const { return stats.pages_free; }

private:
	void bitmap_set(uint64_t page, bool used);

	Bitmap bitmap;

	struct {
		uint64_t pages_free;
		uint64_t pages_total;
	} stats;

	bool initialized = false;
};

extern KernelVMM kernel_vmm;
