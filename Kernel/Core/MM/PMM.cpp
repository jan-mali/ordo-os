#include "limine.h"
#include <Core/Panic.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <Core/MM/PMM.hpp>
#include <cstddef>
#include <Core/memory.hpp>

PMM pmm;

static const char* memmap_entry_types[] = {
    "USABLE",
    "RESERVED",
    "ACPI_RECLAIMABLE",
    "ACPI_NVS",
    "BAD_MEMORY",
    "BOOTLOADER_RECLAIMABLE",
    "EXECUTABLE_AND_MODULES",
    "FRAMEBUFFER",
    "RESERVED_MAPPED"
};

void PMM::init(limine_memmap_response* memmap, limine_hhdm_response* hhdm) {
    if (initialized) {
        panic("PMM: Already initialized.\n");
    }

    if (!memmap) {
        panic("PMM: No memory map received.\n");
    }


    if (!hhdm) {
        panic("PMM: No HHDM response received.\n");
    }

    hhdm_offset = hhdm->offset;

    Logger::log_info("HHDM offset is %X.\n", hhdm_offset);

    uint64_t free_memory = 0;

    uint64_t largest_cont_at = 0;
    uint64_t largest_cont_size = 0;

    uint64_t as_size = 0;

    Logger::log_info("System memory map:\n");
    for (size_t i = 0; i < memmap->entry_count; i++) {
        limine_memmap_entry* entry = memmap->entries[i];

        if (entry->base + entry->length > as_size) {
            as_size = entry->base + entry->length;
        }
        
        Logger::log_info("\tbase=%X, length=%X, type=%d", entry->base, entry->length, entry->type);
        Logger::kprintf("\t %d KiB %s\n", entry->length / 1024, memmap_entry_types[entry->type]);

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            free_memory += entry->length;
            if (entry->length > largest_cont_size) {
                largest_cont_at = entry->base;
                largest_cont_size = entry->length;
            }
        }
    }

    Logger::log_info("Free memory: %d KiB\n", free_memory / 1024);
    Logger::log_info("Largest contiguous block is %d KiB at 0x%X\n", largest_cont_size / 1024, largest_cont_at);

    Logger::log_info("The Address Space is %d KiB large, this is equivalent to:\n", as_size / 1024);

    uint64_t as_pages = (as_size + 4095) / 4096;
    uint64_t pmm_bitmap_bytes = (as_pages + 7) / 8;
    uint64_t pmm_bitmap_pages = (pmm_bitmap_bytes + 4095) / 4096;

    bitmap_length = pmm_bitmap_bytes;
    address_space_pages = as_pages;

    Logger::log_info("\t- %d pages\n", as_pages);
    Logger::log_info("\t- %d KiB PMM bitmap\n", pmm_bitmap_bytes / 1024);
    Logger::log_info("\t- %d PMM bitmap pages\n", pmm_bitmap_pages);

    if (largest_cont_size < pmm_bitmap_bytes) {
        panic("PMM: Largest contiguous block is too small to fit the bitmap. (%D B < %D B)\n", largest_cont_size, pmm_bitmap_bytes);
    }

    bitmap = reinterpret_cast<uint8_t*>(hhdm->offset + largest_cont_at);

    // Mark all memory as used in the beginning
    memset(bitmap, 0xFF, pmm_bitmap_bytes);
    stats.pages_total = (free_memory + 4095) / 4096;
    stats.pages_free = 0;

    for (size_t i = 0; i < memmap->entry_count; i++) {
        limine_memmap_entry* entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            bitmap_set_range(entry->base, entry->length, false);
        }
    }

    bitmap_set_range(largest_cont_at, pmm_bitmap_bytes, true);

    // Reserve page 0 for clean page faults just in case it was usable before
    bitmap_set(0, true);

    initialized = true;

    Logger::log_info("PMM is intialized.\n");
    Logger::log_info("Total usable pages: %D. Total free pages: %D\n", stats.pages_total, stats.pages_free);
}

void* PMM::alloc(uint64_t count) {
    if (!initialized) {
        panic("PMM: Not yet initialized.\n");
    }
    for (uint64_t start = 0; start + count <= address_space_pages; start++) {
        bool all_free = true;
        for (uint64_t p = 0; p < count; p++) {
            if (bitmap_get(start + p)) {
                all_free = false;
                break;
            }
        }

        // Found a viable window
        if (all_free) {
            for (uint64_t p = 0; p < count; p++)
                bitmap_set(start + p, true);

            return reinterpret_cast<void*>((uint64_t)start * 4096);
        }
    }

    panic("PMM: Out of memory! Unable to allocate %D contiguous pages.\n", count);
}

void PMM::free(const void* ptr, uint64_t count) {
    if (!initialized) {
        panic("PMM: Not yet initialized.\n");
    }

    if (count == 0)
        return;

    uint64_t start_page = reinterpret_cast<uint64_t>(ptr) >> 12;

    for (uint64_t p = 0; p < count; p++) {
        if (!bitmap_get(start_page + p)) {
            panic("PMM: Possible double free of physical page %X.\n", start_page + p);
        }
    }

    for (uint64_t p = 0; p < count; p++) {
        bitmap_set(start_page + p, false);
    }

}

void PMM::bitmap_set(uint64_t page, bool used) {
    uint64_t bitmap_byte = page >> 3;
    uint8_t bitmap_bit = page & 7;

    if (used == bitmap_get(page))
        return;


    if (used) {
        stats.pages_free--;
        bitmap[bitmap_byte] |= (1 << bitmap_bit);
    } else {
        stats.pages_free++;
        bitmap[bitmap_byte] &= ~(1 << bitmap_bit);
    }
}

void PMM::bitmap_set_range(uint64_t base, uint64_t length, bool used) {
    uint64_t page_index = base >> 12;
    uint64_t num_pages = (length + 4095) / 4096;

    // TODO: Make this way faster (check if whole words can be marked as used)
    for (uint64_t page = 0; page < num_pages; page++) {
        bitmap_set(page_index + page, used);
    }
}

bool PMM::bitmap_get(uint64_t page) {
    uint64_t bitmap_byte = page >> 3;
    uint8_t bitmap_bit = page & 7;

    return bitmap[bitmap_byte] & (1 << bitmap_bit);
}
