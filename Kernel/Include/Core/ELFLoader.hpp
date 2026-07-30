#pragma once

#include <Core/MM/Paging.hpp>
#include <Core/Result.hpp>
#include <cstdint>

struct ELFImage {
	uint64_t entry;
};

Result<void> load_elf(const char* path, Paging::AddressSpace& address_space, ELFImage& image);
