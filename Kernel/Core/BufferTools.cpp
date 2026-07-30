#include <Core/BufferTools.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Result.hpp>
#include <Core/memory.hpp>
#include <stddef.h>
#include <stdint.h>

namespace BufferTools {

static bool is_user_range(uint64_t addr, size_t len) {
	if (len == 0) {
		return true;
	}

	uint64_t end = addr + len - 1;

	// overflow
	if (end < addr) {
		return false;
	}

	return end < 0x0000800000000000;
}

static bool is_user_buffer_valid(const void* ptr, size_t length, bool write) {
	uint64_t start = reinterpret_cast<uint64_t>(ptr);
	uint64_t end = start + length;

	// overflow
	if (end < start) {
		return false;
	}

	uint64_t page_start = start & ~(Paging::PAGE_SIZE - 1);

	for (uint64_t addr = page_start; addr < end; addr += Paging::PAGE_SIZE) {
		uint64_t flags = Paging::current_address_space->flags(addr);

		if (!(flags & static_cast<uint64_t>(Paging::PageFlags::Present))) {
			// Page not present
			return false;
		}

		if (!(flags & static_cast<uint64_t>(Paging::PageFlags::User))) {
			// Page not user accessible
			return false;
		}

		if (write && !(flags & static_cast<uint64_t>(Paging::PageFlags::Writable))) {
			// Page not writable
			return false;
		}
	}

	return true;
}

Result<void> copy_from_user(void* kernel_dst, const void* user_src, size_t size) {
	if (!is_user_range(reinterpret_cast<uint64_t>(user_src), size)) {
		return Errors::invalid_address();
	}

	if (!is_user_buffer_valid(user_src, size, false)) {
		return Errors::invalid_address();
	}

	memcpy(kernel_dst, user_src, size);

	return {};
}

Result<void> copy_to_user(void* user_dst, const void* kernel_src, size_t size) {
	if (!is_user_range(reinterpret_cast<uint64_t>(user_dst), size)) {
		return Errors::invalid_address();
	}

	if (!is_user_buffer_valid(user_dst, size, true)) {
		return Errors::invalid_address();
	}

	memcpy(user_dst, kernel_src, size);

	return {};
}

Result<size_t> strncpy_from_user(char* kernel_dst, const char* user_src, size_t max_length) {
	for (size_t i = 0; i < max_length; i++) {
		// FIXME: This is REALLY slow

		TRY(copy_from_user(&kernel_dst[i], &user_src[i], 1));

		if (kernel_dst[i] == 0) {
			return i;
		}
	}

	return Error {
		.domain = ErrorDomain::Memory,
		.code = ErrorCode::StringTooLong
	};
}

}
