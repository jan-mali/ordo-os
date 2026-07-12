#pragma once

namespace Logger {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

void log_debug(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_warning(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_critical(const char* fmt, ...);

}
