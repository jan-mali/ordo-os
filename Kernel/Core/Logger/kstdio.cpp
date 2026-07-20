#include <Arch/x86_64/Serial.hpp>
#include <Core/TextConsole.hpp>
#include <Logger/kstdio.hpp>
#include <Utility/String.hpp>
#include <cstdint>

namespace Logger {

static void putc(char c) {
	serial.putc(c);
	text_console.putc(c);
}

static void print_str(const char* str) {
	serial.write(str);
	text_console.write(str);
}

static void print_padding(char c, int count) {
	while (count-- > 0)
		putc(c);
}

static int u64_length(uint64_t value) {
	int length = 1;

	while (value >= 10) {
		value /= 10;
		length++;
	}

	return length;
}

static int hex_length(uint64_t value) {
	int length = 1;

	while (value >= 0x10) {
		value >>= 4;
		length++;
	}

	return length;
}

static void print_hex_raw(uint64_t value, bool uppercase) {
	char buffer[16];
	int i = 0;

	if (value == 0) {
		putc('0');
		return;
	}

	while (value > 0) {
		uint8_t digit = value & 0xF;

		if (digit < 10)
			buffer[i++] = '0' + digit;
		else if (uppercase)
			buffer[i++] = 'A' + digit - 10;
		else
			buffer[i++] = 'a' + digit - 10;

		value >>= 4;
	}

	while (i-- > 0)
		putc(buffer[i]);
}

static void print_dec_raw(uint64_t value) {
	char buffer[20];
	int i = 0;

	if (value == 0) {
		putc('0');
		return;
	}

	while (value > 0) {
		buffer[i++] = '0' + (value % 10);
		value /= 10;
	}

	while (i-- > 0)
		putc(buffer[i]);
}

static void print_hex(uint64_t value, int width, bool zero_pad, bool uppercase) {
	int length = hex_length(value);

	if (zero_pad)
		print_padding('0', width - length);
	else
		print_padding(' ', width - length);

	print_hex_raw(value, uppercase);
}

static void print_unsigned(uint64_t value, int width, bool zero_pad) {
	int length = u64_length(value);

	if (zero_pad)
		print_padding('0', width - length);
	else
		print_padding(' ', width - length);

	print_dec_raw(value);
}

static void print_signed(int64_t value, int width, bool zero_pad) {
	bool negative = value < 0;

	uint64_t absolute;

	if (negative) {
		if (value == INT64_MIN)
			absolute = 9223372036854775808ULL;
		else
			absolute = -value;
	} else {
		absolute = value;
	}

	int length = u64_length(absolute) + (negative ? 1 : 0);

	if (zero_pad && negative) {
		putc('-');
		print_padding('0', width - length);
		print_dec_raw(absolute);
		return;
	}

	if (!zero_pad)
		print_padding(' ', width - length);

	if (negative)
		putc('-');

	print_dec_raw(absolute);
}

static void print_guid(const uint8_t* guid, bool uppercase) {
	const char* hex = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

	for (int i = 3; i >= 0; i--) {
		putc(hex[(guid[i] >> 4) & 0xF]);
		putc(hex[guid[i] & 0xF]);
	}

	putc('-');

	for (int i = 5; i >= 4; i--) {
		putc(hex[(guid[i] >> 4) & 0xF]);
		putc(hex[guid[i] & 0xF]);
	}

	putc('-');

	for (int i = 7; i >= 6; i--) {
		putc(hex[(guid[i] >> 4) & 0xF]);
		putc(hex[guid[i] & 0xF]);
	}

	putc('-');

	for (int i = 8; i < 10; i++) {
		putc(hex[(guid[i] >> 4) & 0xF]);
		putc(hex[guid[i] & 0xF]);
	}

	putc('-');

	for (int i = 10; i < 16; i++) {
		putc(hex[(guid[i] >> 4) & 0xF]);
		putc(hex[guid[i] & 0xF]);
	}
}

static void print_utf16le(const uint16_t* str) {
	while (*str) {
		uint16_t c = *str++;

		if (c < 0x80) {
			putc((char)c);
		} else {
			putc('?');
		}
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

	for (uint32_t i = 0; i < len; i++) {
		char c = fmt[i];

		if (c != '%') {
			putc(c);
			continue;
		}

		i++;

		if (fmt[i] == '%') {
			putc('%');
			continue;
		}

		bool zero_pad = false;
		bool left_align = false;
		int width = 0;

		if (fmt[i] == '0') {
			zero_pad = true;
			i++;
		}

		if (fmt[i] == '-') {
			left_align = true;
			zero_pad = false;
			i++;
		}

		while (fmt[i] >= '0' && fmt[i] <= '9') {
			width = width * 10 + (fmt[i] - '0');
			i++;
		}

		enum {
			NONE,
			HH,
			H,
			L,
			LL
		} modifier = NONE;

		if (fmt[i] == 'h') {
			i++;

			if (fmt[i] == 'h')
				modifier = HH;
			else {
				modifier = H;
				i--;
			}

			i++;
		} else if (fmt[i] == 'l') {
			i++;

			if (fmt[i] == 'l')
				modifier = LL;
			else {
				modifier = L;
				i--;
			}

			i++;
		}

		c = fmt[i];

		switch (c) {

		case 's': {
			const char* value = va_arg(args, const char*);
			value = value ? value : "(null)";

			int length = strlen(value);

			if (!left_align)
				print_padding(' ', width - length);

			print_str(value);

			if (left_align)
				print_padding(' ', width - length);

			break;
		}

		case 'p': {
			void* value = va_arg(args, void*);

			print_hex(reinterpret_cast<uint64_t>(value), 16, true, true);

			break;
		}

		case 'x':
		case 'X': {
			uint64_t value;

			if (modifier == LL || modifier == L)
				value = va_arg(args, uint64_t);
			else
				value = va_arg(args, uint32_t);

			print_hex(value, width, zero_pad, c == 'X');

			break;
		}

		case 'd': {
			int64_t value;

			if (modifier == LL || modifier == L)
				value = va_arg(args, int64_t);
			else
				value = va_arg(args, int32_t);

			print_signed(value, width, zero_pad);

			break;
		}

		case 'u': {
			uint64_t value;

			if (modifier == LL || modifier == L)
				value = va_arg(args, uint64_t);
			else
				value = va_arg(args, uint32_t);

			print_unsigned(value, width, zero_pad);

			break;
		}

		case 'g':
		case 'G': {
			const uint8_t* guid = va_arg(args, const uint8_t*);

			print_guid(guid, c == 'G');

			break;
		}

		case 'w': {
			const uint16_t* value = va_arg(args, const uint16_t*);

			if (value)
				print_utf16le(value);
			else
				print_str("(null)");

			break;
		}

		case 'c': {
			char value = va_arg(args, int) & 0xFF;

			putc(value);

			break;
		}

		default:
			putc('%');
			putc(c);
			break;
		}
	}

	return 0;
}

}
