#include <Core/MM/KernelVMM.hpp>
#include <Core/MM/PMM.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Panic.hpp>
#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Utility/Spinlock.hpp>

namespace Paging {

AddressSpace kernel_address_space;
AddressSpace* current_address_space;

Spinlock paging_spinlock;

static uint64_t* next_table(uint64_t* entry, bool create) {
	if (*entry & static_cast<uint64_t>(PageFlags::Present)) {
		return reinterpret_cast<uint64_t*>(
			pmm.translate_to_hhdm(*entry & ~0xFFFULL)
		);
	}

	if (!create) {
		return nullptr;
	}

	uint64_t page = reinterpret_cast<uint64_t>(pmm.alloc(1));
	if (!page) {
		panic("Paging: Failed to allocate page table\n");
	}

	memset(reinterpret_cast<void*>(pmm.translate_to_hhdm(page)), 0, PAGE_SIZE);

	*entry = page | static_cast<uint64_t>(PageFlags::Present) | static_cast<uint64_t>(PageFlags::Writable) | static_cast<uint64_t>(PageFlags::User);

	return reinterpret_cast<uint64_t*>(
		pmm.translate_to_hhdm(page)
	);
}

AddressSpace::AddressSpace(AddressSpace&& other)
	: pml4_physical(other.pml4_physical) {
	other.pml4_physical = 0;
}

AddressSpace& AddressSpace::operator=(AddressSpace&& other) {
	if (this == &other) {
		return *this;
	}
	pml4_physical = other.pml4_physical;
	other.pml4_physical = 0;

	return *this;
}

void AddressSpace::activate() {
	SpinlockGuard guard(paging_spinlock);

	asm volatile("mov %0, %%cr3" : : "r"(pml4_physical) : "memory");
	current_address_space = this;
}

bool AddressSpace::map(uint64_t virt, uint64_t phys, uint64_t flags) {
	SpinlockGuard guard(paging_spinlock);

	uint64_t* pml4 = reinterpret_cast<uint64_t*>(pmm.translate_to_hhdm(pml4_physical));

	uint64_t pml4_index = (virt >> 39) & 0x1FF;
	uint64_t pdpt_index = (virt >> 30) & 0x1FF;
	uint64_t pd_index = (virt >> 21) & 0x1FF;
	uint64_t pt_index = (virt >> 12) & 0x1FF;

	uint64_t* pdpt = next_table(&pml4[pml4_index], true);
	uint64_t* pd = next_table(&pdpt[pdpt_index], true);
	uint64_t* pt = next_table(&pd[pd_index], true);

	pt[pt_index] = phys | flags | static_cast<uint64_t>(PageFlags::Present);

	asm volatile("invlpg (%0)" ::"r"(virt) : "memory");

	return true;
}

void AddressSpace::unmap(uint64_t virt) {
	SpinlockGuard guard(paging_spinlock);

	uint64_t* pml4 = reinterpret_cast<uint64_t*>(pmm.translate_to_hhdm(pml4_physical));

	uint64_t pml4_index = (virt >> 39) & 0x1FF;
	uint64_t pdpt_index = (virt >> 30) & 0x1FF;
	uint64_t pd_index = (virt >> 21) & 0x1FF;
	uint64_t pt_index = (virt >> 12) & 0x1FF;

	uint64_t* pdpt = next_table(&pml4[pml4_index], false);
	if (!pdpt)
		return;

	uint64_t* pd = next_table(&pdpt[pdpt_index], false);
	if (!pd)
		return;

	uint64_t* pt = next_table(&pd[pd_index], false);
	if (!pt)
		return;

	pt[pt_index] = 0;

	asm volatile("invlpg (%0)" ::"r"(virt) : "memory");
}

bool AddressSpace::map_mmio_to_hhdm(uint64_t phys) {
	return map(pmm.translate_to_hhdm(phys), phys, static_cast<uint64_t>(PageFlags::Writable) | static_cast<uint64_t>(PageFlags::PCD) | static_cast<uint64_t>(PageFlags::PWT) | static_cast<uint64_t>(PageFlags::NX));
}

bool AddressSpace::map_pci_bar_to_hhdm(uint64_t address, uint64_t size) {
	uint64_t virt = pmm.get_hhdm_offset() + address;

	// Make sure the HHDM mapping does not collide with the kernel heap
	if (virt + size < virt) {
		panic("PCI BAR: address overflow\n");
	}

	if (virt < KERNEL_HEAP_START && virt + size > KERNEL_HEAP_START) {
		panic("PCI BAR overlaps kernel heap\n");
	}

	uint64_t aligned_phys = address & ~0xFFF;
	uint64_t aligned_virt = virt & ~0xFFF;

	uint64_t offset = address & 0xFFF;
	uint64_t pages = (offset + size + 0xFFF) / 0x1000;

	for (uint64_t i = 0; i < pages; i++) {
		if (!map(aligned_virt + (i * 0x1000), aligned_phys + (i * 0x1000), static_cast<uint64_t>(PageFlags::Writable) | static_cast<uint64_t>(PageFlags::PCD) | static_cast<uint64_t>(PageFlags::PWT) | static_cast<uint64_t>(PageFlags::NX))) {
			return false;
		}
	}

	return true;
}

bool AddressSpace::is_mapped(uint64_t virt) const {
	SpinlockGuard guard(paging_spinlock);

	uint64_t* pml4 = reinterpret_cast<uint64_t*>(pmm.translate_to_hhdm(pml4_physical));

	uint64_t pml4_index = (virt >> 39) & 0x1FF;
	uint64_t pdpt_index = (virt >> 30) & 0x1FF;
	uint64_t pd_index = (virt >> 21) & 0x1FF;
	uint64_t pt_index = (virt >> 12) & 0x1FF;

	uint64_t* pdpt = next_table(&pml4[pml4_index], false);
	if (!pdpt)
		return false;

	uint64_t* pd = next_table(&pdpt[pdpt_index], false);
	if (!pd)
		return false;

	uint64_t* pt = next_table(&pd[pd_index], false);
	if (!pt)
		return false;

	return pt[pt_index] & static_cast<uint64_t>(PageFlags::Present);
}

uint64_t AddressSpace::flags(uint64_t virt) const {
	SpinlockGuard guard(paging_spinlock);

	uint64_t* pml4 = reinterpret_cast<uint64_t*>(pmm.translate_to_hhdm(pml4_physical));

	uint64_t pml4_index = (virt >> 39) & 0x1FF;
	uint64_t pdpt_index = (virt >> 30) & 0x1FF;
	uint64_t pd_index = (virt >> 21) & 0x1FF;
	uint64_t pt_index = (virt >> 12) & 0x1FF;

	uint64_t* pdpt = next_table(&pml4[pml4_index], false);
	if (!pdpt)
		return 0;

	uint64_t* pd = next_table(&pdpt[pdpt_index], false);
	if (!pd)
		return 0;

	uint64_t* pt = next_table(&pd[pd_index], false);
	if (!pt)
		return 0;

	return pt[pt_index] & 0xFFF;
}

uint64_t AddressSpace::translate(uint64_t virt) const {
	SpinlockGuard guard(paging_spinlock);

	uint64_t* pml4 = reinterpret_cast<uint64_t*>(pmm.translate_to_hhdm(pml4_physical));

	uint64_t i4 = (virt >> 39) & 0x1FF;
	uint64_t i3 = (virt >> 30) & 0x1FF;
	uint64_t i2 = (virt >> 21) & 0x1FF;
	uint64_t i1 = (virt >> 12) & 0x1FF;

	if (!(pml4[i4] & static_cast<uint64_t>(PageFlags::Present)))
		return 0;

	auto* pdpt = reinterpret_cast<uint64_t*>(
		pmm.translate_to_hhdm(pml4[i4] & ~0xFFFULL)
	);

	if (!(pdpt[i3] & static_cast<uint64_t>(PageFlags::Present)))
		return 0;

	auto* pd = reinterpret_cast<uint64_t*>(
		pmm.translate_to_hhdm(pdpt[i3] & ~0xFFFULL)
	);

	if (!(pd[i2] & static_cast<uint64_t>(PageFlags::Present)))
		return 0;

	auto* pt = reinterpret_cast<uint64_t*>(
		pmm.translate_to_hhdm(pd[i2] & ~0xFFFULL)
	);

	if (!(pt[i1] & static_cast<uint64_t>(PageFlags::Present)))
		return 0;

	return (pt[i1] & ~0xFFFULL) | (virt & 0xFFF);
}

uint64_t AddressSpace::translate_hhdm(uint64_t virt) const {
	return pmm.translate_to_hhdm(translate(virt));
}

AddressSpace AddressSpace::create_kernel_address_space() {
	SpinlockGuard guard(paging_spinlock);

	uint64_t pml4_phys = reinterpret_cast<uint64_t>(pmm.alloc(1));
	if (!pml4_phys)
		panic("Failed to allocate PML4");

	auto* new_pml4 = reinterpret_cast<uint64_t*>(pmm.translate_to_hhdm(pml4_phys));

	memset(new_pml4, 0, PAGE_SIZE);

	uint64_t cr3;
	asm volatile(
		"mov %%cr3, %0"
		: "=r"(cr3)
		:
		: "memory"
	);

	uint64_t limine_pml4_phys = cr3 & ~0xFFFULL;
	auto* limine_pml4 = reinterpret_cast<uint64_t*>(pmm.translate_to_hhdm(limine_pml4_phys));

	for (size_t i = 256; i < 512; i++) {
		// Copy Limine PDPT entries to the new PML4
		new_pml4[i] = limine_pml4[i];

		// If the entry doesn't exist, create a new PDPT for the kernel space
		if (!(new_pml4[i] & static_cast<uint64_t>(PageFlags::Present))) {
			next_table(&new_pml4[i], true);
		}
	}

	return AddressSpace(pml4_phys);
}

void init() {
	kernel_address_space = AddressSpace::create_kernel_address_space();
	kernel_address_space.activate();

	Logger::log_info("Paging is initialized.\n");
}

}
