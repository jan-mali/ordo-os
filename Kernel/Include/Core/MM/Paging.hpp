#pragma once

#include <cstdint>

namespace Paging {

constexpr uint64_t PRESENT = 1 << 0;
constexpr uint64_t WRITABLE = 1 << 1;
constexpr uint64_t USER = 1 << 2;
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
