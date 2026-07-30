#pragma once

#include <Core/Panic.hpp>
#include <cstdint>

enum class ErrorDomain : uint8_t {
	Generic,

	Memory,
	Process,
	Thread,
	Scheduler,

	VFS,
	FileSystem,

	Device,
	Driver,
	PCI,

	ELF,
	Exec,

	IPC,
	Network,

	ACPI,
};

enum class ErrorCode : uint16_t {
	GenericError,

	// Generic
	InvalidArgument,
	InvalidAddress,
	InvalidHandle,
	PermissionDenied,
	NotImplemented,
	OutOfMemory,
	Busy,
	Timeout,

	// Object state
	NotFound,
	AlreadyExists,
	IsDirectory,
	NotDirectory,
	NotEmpty,

	// I/O
	ReadError,
	WriteError,
	EndOfFile,

	// User/kernel boundary
	BufferTooSmall,
	StringTooLong,

	// ELF
	InvalidELF,
	UnsupportedABI,
};

struct Error {
	ErrorDomain domain;
	ErrorCode code;
};

template<typename T>
class Result {
public:
	Result(T&& value) {
		m_has_value = true;
		m_value = static_cast<T&&>(value);
	}

	Result(const T& value) {
		m_has_value = true;
		m_value = value;
	}

	Result(Error error) {
		m_has_value = false;
		m_error = error;
	}

	bool has_value() const {
		return m_has_value;
	}

	explicit operator bool() const {
		return m_has_value;
	}

	T& value() {
		return m_value;
	}

	const T& value() const {
		return m_value;
	}

	Error error() const {
		return m_error;
	}

	bool is_error(ErrorCode code) const {
		return !m_has_value && m_error.code == code;
	}

	T value_or(T default_value) const {
		return m_has_value ? m_value : default_value;
	}

private:
	union {
		T m_value;
		Error m_error;
	};

	bool m_has_value;
};

template<>
class Result<void> {
public:
	Result() {
		m_success = true;
	}

	Result(Error error) {
		m_success = false;
		m_error = error;
	}

	explicit operator bool() const {
		return m_success;
	}

	void value() const {
		if (!m_success) {
			panic("Result<void> has no value");
		}
	}

	Error error() const {
		return m_error;
	}

	bool is_error(ErrorCode code) const {
		return !m_success && m_error.code == code;
	}

private:
	bool m_success;
	Error m_error;
};

#define TRY(expr) ({        \
	auto __r = (expr);      \
	if (!__r)               \
		return __r.error(); \
	__r.value();            \
})

#define MUST(expr) ({                      \
	auto __r = (expr);                     \
	if (!__r)                              \
		panic("MUST(" #expr ") failed\n"); \
	__r.value();                           \
})

namespace Errors {

inline Error not_found(ErrorDomain d) {
	return { d, ErrorCode::NotFound };
}

inline Error invalid_address() {
	return { ErrorDomain::Memory, ErrorCode::InvalidAddress };
}

inline Error out_of_memory() {
	return { ErrorDomain::Memory, ErrorCode::OutOfMemory };
}

inline Error not_implemented() {
	return { ErrorDomain::Generic, ErrorCode::NotImplemented };
}

}
