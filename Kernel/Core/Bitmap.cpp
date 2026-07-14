#include <Core/Bitmap.hpp>
#include <Core/Panic.hpp>

void Bitmap::set(uint64_t page, bool used) {
	if (!initialized) {
		panic("Bitmap: Not yet initialized.\n");
	}

	uint64_t bitmap_byte = page >> 3;
	uint8_t bitmap_bit = page & 7;

	if (used == get(page))
		return;

	if (used) {
		data[bitmap_byte] |= (1 << bitmap_bit);
	} else {
		data[bitmap_byte] &= ~(1 << bitmap_bit);
	}
}

void Bitmap::set_range(uint64_t base, uint64_t length, bool used) {
	if (!initialized) {
		panic("Bitmap: Not yet initialized.\n");
	}

	uint64_t page_index = base >> 12;
	uint64_t num_pages = (length + 4095) / 4096;

	// TODO: Make this way faster (check if whole words can be marked as used)
	for (uint64_t page = 0; page < num_pages; page++) {
		set(page_index + page, used);
	}
}

void Bitmap::set_all(bool used) {
	if (!initialized) {
		panic("Bitmap: Not yet initialized.\n");
	}

	uint8_t value = used ? 0xFF : 0x00;
	for (uint64_t i = 0; i < length; i++) {
		data[i] = value;
	}
}

bool Bitmap::get(uint64_t page) {
	if (!initialized) {
		panic("Bitmap: Not yet initialized.\n");
	}

	uint64_t bitmap_byte = page >> 3;
	uint8_t bitmap_bit = page & 7;

	return data[bitmap_byte] & (1 << bitmap_bit);
}
