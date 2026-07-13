#include <Core/TextConsole.hpp>
#include <Arch/x86_64/Serial.hpp>
#include <Logger/kstdio.hpp>
#include <Logger/Logger.hpp>
#include <stdarg.h>

namespace Logger {

static void vlog(LogLevel level, const char* fmt, va_list args) {
    // TODO: Extend this to show the current task
    if (false) {
        serial.write("\x1b[93m");
        text_console.set_color(0xfff176, 0);
        kprintf("some-process(some-pid)");
        serial.write("\x1b[0m");
        text_console.set_color(0xffffff, 0);
    } else {
        serial.write("\x1b[96m");
        text_console.set_color(0x00e5ff, 0);
        kprintf("Kernel");
        serial.write("\x1b[0m");
        text_console.set_color(0xffffff, 0);
    }

    kprintf(": ");
    switch (level) {
    case LogLevel::Debug:
        serial.write("\x1b[35m");
        text_console.set_color(0xab47bc, 0);
        kprintf("DEBUG");
        serial.write("\x1b[0m");
        text_console.set_color(0xffffff, 0);
        break;
    case LogLevel::Info:
        serial.write("\x1b[32m");
        text_console.set_color(0x4caf50, 0);
        kprintf("INFO");
        serial.write("\x1b[0m");
        text_console.set_color(0xffffff, 0);
        break;
    case LogLevel::Warning:
        serial.write("\x1b[33m");
        text_console.set_color(0xffd54f, 0);
        kprintf("WARNING");
        serial.write("\x1b[0m");
        text_console.set_color(0xffffff, 0);
        break;
    case LogLevel::Error:
        serial.write("\x1b[31m");
        text_console.set_color(0xef5350, 0);
        kprintf("ERROR");
        serial.write("\x1b[0m");
        text_console.set_color(0xffffff, 0);
        break;
    case LogLevel::Critical:
        serial.write("\x1b[37;41m");
        text_console.set_color(0xffffff, 0xef5350);
        kprintf("CRITICAL");
        serial.write("\x1b[0m");
        text_console.set_color(0xffffff, 0);
        break;
    default:
        serial.write("\x1b[0m");
        text_console.set_color(0, 0xffffff);
        kprintf("UNKNOWN");
        serial.write("\x1b[0m");
        text_console.set_color(0xffffff, 0);
        break;
    }

    kprintf(" ");

    kvprintf(fmt, args);
}

void log_debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(LogLevel::Debug, fmt, args);

    va_end(args);
}

void log_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(LogLevel::Info, fmt, args);
    
    va_end(args);
}

void log_warning(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(LogLevel::Warning, fmt, args);
    
    va_end(args);
}

void log_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(LogLevel::Error, fmt, args);
    
    va_end(args);
}

void log_critical(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(LogLevel::Critical, fmt, args);
    
    va_end(args);
}

}
