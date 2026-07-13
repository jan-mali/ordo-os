#pragma once

#include <limine.h>
#include <cstdint>

class PMM {
public:
    void init(limine_memmap_response* memmap, limine_hhdm_response* hhdm);

    void* alloc(uint64_t count);
    void free(const void* ptr, uint64_t count);

    uint64_t get_total_pages() const { return stats.pages_total; }
    uint64_t get_free_pages() const { return stats.pages_free; }
private:
    void bitmap_set(uint64_t page, bool used);
    void bitmap_set_range(uint64_t base, uint64_t length, bool used);
    bool bitmap_get(uint64_t page);

    uint8_t* bitmap;

    struct {
        uint64_t pages_free;
        uint64_t pages_total;
    } stats;

    uint64_t bitmap_length;
    uint64_t address_space_pages;
    uint64_t hhdm_offset;

    bool initialized = false;
};

extern PMM pmm;
