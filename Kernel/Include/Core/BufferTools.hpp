#pragma once

#include <Core/Result.hpp>
#include <stddef.h>

namespace BufferTools {

Result<void> copy_from_user(void* kernel_dst, const void* user_src, size_t size);
Result<void> copy_to_user(void* user_dst, const void* kernel_src, size_t size);

Result<size_t> strncpy_from_user(char* kernel_dst, const char* user_src, size_t max_length);

}
