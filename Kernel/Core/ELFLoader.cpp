#include <Core/ELFLoader.hpp>
#include <Core/MM/PMM.hpp>
#include <Core/Result.hpp>
#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Utility/String.hpp>
#include <VFS/OpenFile.hpp>
#include <VFS/VFS.hpp>
#include <cstdint>

constexpr uint8_t EI_NIDENT = 16;

// e_ident indexes
constexpr uint8_t EI_MAG0 = 0;
constexpr uint8_t EI_MAG1 = 1;
constexpr uint8_t EI_MAG2 = 2;
constexpr uint8_t EI_MAG3 = 3;
constexpr uint8_t EI_CLASS = 4;
constexpr uint8_t EI_DATA = 5;
constexpr uint8_t EI_VERSION = 6;

// Magic
constexpr uint8_t ELFMAG0 = 0x7f;
constexpr uint8_t ELFMAG1 = 'E';
constexpr uint8_t ELFMAG2 = 'L';
constexpr uint8_t ELFMAG3 = 'F';

// File class
constexpr uint8_t ELFCLASS64 = 2;

// Endianness
constexpr uint8_t ELFDATA2LSB = 1;

// Version
constexpr uint8_t EV_CURRENT = 1;

// Types
constexpr uint16_t ET_EXEC = 2;
constexpr uint16_t ET_DYN = 3;

// Machine
constexpr uint16_t EM_X86_64 = 62;

// Program header types
constexpr uint32_t PT_NULL = 0;
constexpr uint32_t PT_LOAD = 1;

// Segment flags
constexpr uint32_t PF_X = 1;
constexpr uint32_t PF_W = 2;
constexpr uint32_t PF_R = 4;

// ELF header
struct Elf64_Ehdr {
	unsigned char e_ident[EI_NIDENT];

	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;

	uint64_t e_entry;

	uint64_t e_phoff;
	uint64_t e_shoff;

	uint32_t e_flags;

	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;

	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

// Program header
struct Elf64_Phdr {
	uint32_t p_type;
	uint32_t p_flags;

	uint64_t p_offset;

	uint64_t p_vaddr;
	uint64_t p_paddr;

	uint64_t p_filesz;
	uint64_t p_memsz;

	uint64_t p_align;
};

Result<void> load_elf(const char* path, Paging::AddressSpace& address_space, ELFImage& image) {
	VFS::OpenFile* elf_file = TRY(VFS::VFS::open(path));

	Elf64_Ehdr header {};

	// TODO: Figure out the error propagation here
	if (MUST(elf_file->read(sizeof(header), reinterpret_cast<uint8_t*>(&header))) != sizeof(header)) {
		VFS::VFS::close(elf_file);
		return Error {
			.domain = ErrorDomain::ELF,
			.code = ErrorCode::EndOfFile
		};
	}

	// ELF magic
	if (header.e_ident[0] != 0x7f || header.e_ident[1] != 'E' || header.e_ident[2] != 'L' || header.e_ident[3] != 'F') {
		VFS::VFS::close(elf_file);
		return Error {
			.domain = ErrorDomain::ELF,
			.code = ErrorCode::InvalidELF
		};
	}

	if (header.e_ident[4] != ELFCLASS64 || header.e_machine != EM_X86_64) {
		VFS::VFS::close(elf_file);
		return Error {
			.domain = ErrorDomain::ELF,
			.code = ErrorCode::UnsupportedABI
		};
	}

	image.entry = header.e_entry;

	for (uint16_t i = 0; i < header.e_phnum; i++) {
		Elf64_Phdr ph {};

		elf_file->offset = header.e_phoff + i * sizeof(Elf64_Phdr);

		// TODO: Figure out the error propagation here
		if (MUST(elf_file->read(sizeof(ph), reinterpret_cast<uint8_t*>(&ph))) != sizeof(ph)) {
			VFS::VFS::close(elf_file);
			return Error {
				.domain = ErrorDomain::ELF,
				.code = ErrorCode::EndOfFile
			};
		}

		if (ph.p_type != PT_LOAD)
			continue;

		uint64_t segment_start = ph.p_vaddr & ~(Paging::PAGE_SIZE - 1);

		uint64_t segment_end = (ph.p_vaddr + ph.p_memsz + Paging::PAGE_SIZE - 1) & ~(Paging::PAGE_SIZE - 1);

		// Allocate and map pages
		for (uint64_t addr = segment_start; addr < segment_end; addr += Paging::PAGE_SIZE) {
			uint64_t phys = reinterpret_cast<uint64_t>(pmm.alloc(1));

			if (!phys) {
				VFS::VFS::close(elf_file);
				return Error {
					.domain = ErrorDomain::ELF,
					.code = ErrorCode::OutOfMemory
				};
			}

			uint64_t flags = static_cast<uint64_t>(Paging::PageFlags::Present) | static_cast<uint64_t>(Paging::PageFlags::User);

			if (ph.p_flags & PF_W) {
				flags |= static_cast<uint64_t>(Paging::PageFlags::Writable);
			}

			// NX unless executable
			if (!(ph.p_flags & PF_X)) {
				flags |= static_cast<uint64_t>(Paging::PageFlags::NX);
			}

			if (!address_space.map(addr, phys, flags)) {
				VFS::VFS::close(elf_file);
				return Error {
					.domain = ErrorDomain::ELF,
					.code = ErrorCode::OutOfMemory
				};
			}

			memset(reinterpret_cast<void*>(pmm.translate_to_hhdm(phys)), 0x42, Paging::PAGE_SIZE);
		}

		// Copy file contents
		uint64_t remaining = ph.p_filesz;
		uint64_t file_offset = ph.p_offset;
		uint64_t virt = ph.p_vaddr;

		while (remaining) {
			uint64_t page = virt & ~(Paging::PAGE_SIZE - 1);
			uint64_t page_offset = virt & (Paging::PAGE_SIZE - 1);
			uint64_t amount = Paging::PAGE_SIZE - page_offset;

			if (amount > remaining)
				amount = remaining;

			uint64_t phys = address_space.translate(virt);

			uint8_t* destination = reinterpret_cast<uint8_t*>(pmm.translate_to_hhdm(phys)) + page_offset;

			elf_file->offset = file_offset;

			// TODO: Figure out the error propagation here
			if (MUST(elf_file->read(amount, destination)) != amount) {
				VFS::VFS::close(elf_file);
				return Error {
					.domain = ErrorDomain::ELF,
					.code = ErrorCode::EndOfFile
				};
			}

			virt += amount;
			file_offset += amount;
			remaining -= amount;
		}
	}

	VFS::VFS::close(elf_file);
	return {};
}
