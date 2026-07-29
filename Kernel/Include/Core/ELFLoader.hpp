#pragma once

#include <Core/MM/Paging.hpp>
#include <cstdint>

struct ELFImage {
	uint64_t entry;
};

bool load_elf(const char* path, Paging::AddressSpace& address_space, ELFImage& image);
