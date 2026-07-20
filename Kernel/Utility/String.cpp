#include <Core/memory.hpp>
#include <Utility/String.hpp>
#include <cstddef>

extern "C" {

size_t strlen(const char* str) {
	size_t len = 0;
	while (*str++) {
		len++;
	}
	return len;
}

int strcmp(const char* a, const char* b) {
	while (*a && (*a == *b)) {
		++a;
		++b;
	}

	return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char* a, const char* b, size_t n) {
	while (n && *a && (*a == *b)) {
		++a;
		++b;
		--n;
	}

	if (n == 0)
		return 0;

	return (unsigned char)*a - (unsigned char)*b;
}

char* strcpy(char* dst, const char* src) {
	char* ret = dst;

	while ((*dst++ = *src++))
		;

	return ret;
}

char* strncpy(char* dst, const char* src, size_t n) {
	char* ret = dst;

	while (n && *src) {
		*dst++ = *src++;
		--n;
	}

	while (n--) {
		*dst++ = '\0';
	}

	return ret;
}

char* strcat(char* dst, const char* src) {
	char* ret = dst;

	while (*dst)
		++dst;

	while ((*dst++ = *src++))
		;

	return ret;
}

char* strchr(const char* str, int c) {
	while (*str) {
		if (*str == (char)c)
			return (char*)str;
		++str;
	}

	return (c == '\0') ? (char*)str : nullptr;
}

char* strrchr(const char* str, int c) {
	const char* last = nullptr;

	while (*str) {
		if (*str == (char)c)
			last = str;
		++str;
	}

	if (c == '\0')
		return (char*)str;

	return (char*)last;
}

char* strdup(const char* str) {
	size_t len = strlen(str);

	char* out = new char[len + 1];
	if (!out)
		return nullptr;

	memcpy(out, str, len + 1);
	return out;
}
}
