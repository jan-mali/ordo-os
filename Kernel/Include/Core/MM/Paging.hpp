#pragma once

#include <cstdint>

namespace Paging {

enum class PageFlags : uint64_t {
	Present = 1ull << 0,
	Writable = 1ull << 1,
	User = 1ull << 2,
	PWT = 1ull << 3,
	PCD = 1ull << 4,
	PAT = 1ull << 7,
	Global = 1ull << 8,
	NX = 1ull << 63,
};

constexpr uint64_t PAGE_SIZE = 4096;

class AddressSpace {
public:
	AddressSpace() = default;
	AddressSpace(uint64_t pml4_physical)
		: pml4_physical(pml4_physical) {}

	AddressSpace(const AddressSpace&) = delete;
	AddressSpace(AddressSpace&&);
	AddressSpace& operator=(const AddressSpace&) = delete;
	AddressSpace& operator=(AddressSpace&&);

	// FIXME: Possibly implement a destructor that frees the PML4 table and all associated page tables.
	~AddressSpace() = default;

	void activate();
	bool map(uint64_t virt, uint64_t phys, uint64_t flags);
	void unmap(uint64_t virt);

	bool map_mmio_to_hhdm(uint64_t phys);
	bool map_pci_bar_to_hhdm(uint64_t address, uint64_t size);

	bool is_mapped(uint64_t virt) const;
	uint64_t flags(uint64_t virt) const;

	uint64_t translate(uint64_t virt) const;
	uint64_t translate_hhdm(uint64_t virt) const;

	static AddressSpace create_kernel_address_space();

private:
	uint64_t pml4_physical;
};

extern AddressSpace kernel_address_space;
extern AddressSpace* current_address_space;

void init();

}
