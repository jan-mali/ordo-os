#include <Core/MM/KernelVMM.hpp>
#include <Core/MM/PMM.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Panic.hpp>
#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <Utility/Spinlock.hpp>
#include <limine.h>

KernelVMM kernel_vmm;
Spinlock vmm_spinlock;

void KernelVMM::init() {
	if (initialized) {
		panic("KernelVMM: Already initialized.\n");
	}

	stats.pages_total = KERNEL_HEAP_PAGES;
	stats.pages_free = KERNEL_HEAP_PAGES;

	uint64_t physical_bitmap_address = reinterpret_cast<uint64_t>(pmm.alloc(KERNEL_HEAP_BITMAP_PAGES));
	uint8_t* bitmap_data = reinterpret_cast<uint8_t*>(pmm.translate_to_hhdm(physical_bitmap_address));

	bitmap.init(bitmap_data, KERNEL_HEAP_BITMAP_PAGES * 4096 / 8);
	bitmap.set_all(false); // Mark all pages as free

	initialized = true;

	Logger::log_info("KernelVMM is intialized.\n");
	Logger::log_info("Total pages: %lld. Total free pages: %lld\n", stats.pages_total, stats.pages_free);
}

void* KernelVMM::allocate_pages(uint64_t count) {
	if (!initialized) {
		panic("KernelVMM: Not yet initialized.\n");
	}

	SpinlockGuard guard(vmm_spinlock);

	for (uint64_t start = 0; start + count <= KERNEL_HEAP_PAGES; start++) {
		bool all_free = true;
		for (uint64_t p = 0; p < count; p++) {
			if (bitmap.get(start + p)) {
				all_free = false;
				break;
			}
		}

		// Found a viable window
		if (all_free) {
			for (uint64_t p = 0; p < count; p++) {
				bitmap_set(start + p, true);

				// Allocate physical pages for the virtual pages
				uint64_t phys_page = reinterpret_cast<uint64_t>(pmm.alloc(1));
				Paging::kernel_address_space.map(start * 4096 + p * 4096 + KERNEL_HEAP_START, phys_page, static_cast<uint64_t>(Paging::PageFlags::Writable));
			}

			// Return the virtual address of the allocated pages
			return reinterpret_cast<void*>((uint64_t)start * 4096 + KERNEL_HEAP_START);
		}
	}

	panic("KernelVMM: Out of memory! Unable to allocate %lld contiguous pages.\n", count);
}

void KernelVMM::free_pages(const void* ptr, uint64_t count) {
	if (!initialized) {
		panic("KernelVMM: Not yet initialized.\n");
	}

	SpinlockGuard guard(vmm_spinlock);

	uint64_t address = reinterpret_cast<uint64_t>(ptr);

	if (address < KERNEL_HEAP_START || address >= KERNEL_HEAP_END || (address - KERNEL_HEAP_START) % 4096 != 0) {
		panic("KernelVMM: Invalid free address %p.\n", ptr);
	}

	if (count == 0)
		return;

	uint64_t start_page = (reinterpret_cast<uint64_t>(ptr) - KERNEL_HEAP_START) >> 12;

	for (uint64_t p = 0; p < count; p++) {
		if (!bitmap.get(start_page + p)) {
			panic("KernelVMM: Possible double free of virtual kernel heap address %p.\n", ptr);
		}
	}

	for (uint64_t p = 0; p < count; p++) {
		uint64_t virt = KERNEL_HEAP_START + (start_page + p) * 4096;
		uint64_t phys = Paging::kernel_address_space.translate(virt);
		Paging::kernel_address_space.unmap(virt);
		pmm.free(reinterpret_cast<void*>(phys), 1);
		bitmap_set(start_page + p, false);
	}
}

void KernelVMM::bitmap_set(uint64_t page, bool used) {
	if (used == bitmap.get(page))
		return;

	if (used) {
		stats.pages_free--;
		bitmap.set(page, true);
	} else {
		stats.pages_free++;
		bitmap.set(page, false);
	}
}
