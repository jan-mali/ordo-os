#pragma once

#include <cstdarg>
namespace Logger {

enum class LogLevel {
	Debug,
	Info,
	Warning,
	Error,
	Critical
};

void vlog(LogLevel level, const char* fmt, va_list args);
void log_debug(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_warning(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_critical(const char* fmt, ...);

}
