#pragma once

#include <cstdint>

class Bitmap {
public:
	Bitmap()
		: data(nullptr)
		, length(0)
		, initialized(false) {}

	Bitmap(uint8_t* data, uint64_t length)
		: data(data)
		, length(length)
		, initialized(true) {}

	void init(uint8_t* data, uint64_t length) {
		this->data = data;
		this->length = length;
		initialized = true;
	}

	void set(uint64_t page, bool used);
	void set_range(uint64_t base, uint64_t length, bool used);
	void set_all(bool used);

	bool get(uint64_t page);

private:
	uint8_t* data;
	uint64_t length;

	bool initialized;
};
