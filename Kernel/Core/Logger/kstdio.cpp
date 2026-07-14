#include <Arch/x86_64/Serial.hpp>
#include <Core/TextConsole.hpp>
#include <Logger/kstdio.hpp>
#include <cstdint>

namespace Logger {

static uint32_t strlen(const char* str) {
	uint32_t len = 0;
	while (*str++) {
		len++;
	}
	return len;
}

static void putc(char c) {
	serial.putc(c);
	text_console.putc(c);
}

static void print_str(const char* str) {
	serial.write(str);
	text_console.write(str);
}

static void print_hex32(uint32_t value) {
	uint32_t divisor = 0x10000000;
	while (divisor > 0) {
		uint32_t digit = value / divisor;
		if (digit >= 10) {
			putc('A' + digit - 10);
		} else {
			putc('0' + digit);
		}
		value -= digit * divisor;
		divisor /= 0x10;
	}
}

static void print_hex64(uint64_t value) {
	uint32_t high = value >> 32;
	uint32_t low = value & 0xFFFFFFFF;
	print_hex32(high);
	print_hex32(low);
}

static void print_sdec32(int32_t value) {
	char buffer[12]; // enough for -2147483648\0
	int i = 0;

	// Handle negative numbers
	if (value < 0) {
		putc('-');
		// Special case for INT32_MIN (−2147483648)
		if (value == INT32_MIN) {
			// INT32_MIN = -2147483648 → print manually
			const char* minstr = "2147483648";
			while (*minstr)
				putc(*minstr++);
			return;
		}
		value = -value;
	}

	// Handle 0 explicitly
	if (value == 0) {
		putc('0');
		return;
	}

	// Convert to string in reverse order
	while (value > 0) {
		buffer[i++] = '0' + (value % 10);
		value /= 10;
	}

	// Print digits in correct order
	while (i-- > 0) {
		putc(buffer[i]);
	}
}

static void print_sdec64(int64_t value) {
	char buffer[21]; // enough for -9223372036854775808\0
	int i = 0;

	// Handle negative numbers
	if (value < 0) {
		putc('-');
		// Special case for INT64_MIN (−9223372036854775808)
		if (value == INT64_MIN) {
			const char* minstr = "9223372036854775808";
			while (*minstr)
				putc(*minstr++);
			return;
		}
		value = -value;
	}

	// Handle 0 explicitly
	if (value == 0) {
		putc('0');
		return;
	}

	// Convert to string in reverse order
	while (value > 0) {
		buffer[i++] = '0' + (value % 10);
		value /= 10;
	}

	// Print digits in correct order
	while (i-- > 0) {
		putc(buffer[i]);
	}
}

int kprintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int ret = kvprintf(fmt, args);

	va_end(args);

	return ret;
}

int kvprintf(const char* fmt, va_list args) {
	uint32_t len = strlen(fmt);

	bool special = false;

	for (uint32_t i = 0; i < len; i++) {
		char c = fmt[i];
		if (c == '%') {
			if (special) {
				putc(c);
			} else {
				special = true;
				continue;
			}
		}

		if (special) {
			special = false;

			if (c == 's') {
				const char* value = va_arg(args, const char*);
				print_str(value ? value : "(null)");
				continue;
			}

			if (c == 'X') {
				uint64_t value = va_arg(args, uint64_t);
				print_hex64(value);
				continue;
			}

			if (c == 'x') {
				uint32_t value = va_arg(args, uint32_t);
				print_hex32(value);
				continue;
			}

			if (c == 'D') {
				int64_t value = va_arg(args, int64_t);
				print_sdec64(value);
				continue;
			}

			if (c == 'd') {
				int32_t value = va_arg(args, int32_t);
				print_sdec32(value);
				continue;
			}

			putc('%');
		}

		putc(c);
	}

	return 0;
}

}
