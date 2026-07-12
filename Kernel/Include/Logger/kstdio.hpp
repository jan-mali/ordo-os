#pragma once

#include <stdarg.h>

namespace Logger {
    int kprintf(const char* fmt, ...);
    int kvprintf(const char* fmt, va_list args);
}
