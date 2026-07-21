#include <Core/memory.hpp>
#include <Drivers/BlockDevice.hpp>

namespace Drivers {

bool BlockDevice::read_bytes(uint64_t address, uint64_t bytes, uint8_t* buffer) {
	if (bytes == 0)
		return true;

	const uint64_t sector_bytes = sector_size();

	uint8_t* sector_buffer = new uint8_t[sector_bytes];

	uint64_t current_address = address;
	uint64_t remaining = bytes;
	uint8_t* out = buffer;

	while (remaining > 0) {
		uint64_t sector = current_address / sector_bytes;
		uint64_t offset = current_address % sector_bytes;

		uint64_t chunk = sector_bytes - offset;
		if (chunk > remaining)
			chunk = remaining;

		if (offset == 0 && chunk == sector_bytes) {
			// Whole-sector read.
			if (!read(sector, 1, out)) {
				delete[] sector_buffer;
				return false;
			}
		} else {
			// Partial-sector read.
			if (!read(sector, 1, sector_buffer)) {
				delete[] sector_buffer;
				return false;
			}

			memcpy(out, sector_buffer + offset, chunk);
		}

		current_address += chunk;
		out += chunk;
		remaining -= chunk;
	}

	delete[] sector_buffer;
	return true;
}

bool BlockDevice::write_bytes(uint64_t address, uint64_t bytes, uint8_t* buffer) {
	if (bytes == 0)
		return true;

	const uint64_t sector_bytes = sector_size();

	uint8_t* sector_buffer = new uint8_t[sector_bytes];

	uint64_t current_address = address;
	uint64_t remaining = bytes;
	uint8_t* in = buffer;

	while (remaining > 0) {
		uint64_t sector = current_address / sector_bytes;
		uint64_t offset = current_address % sector_bytes;

		uint64_t chunk = sector_bytes - offset;
		if (chunk > remaining)
			chunk = remaining;

		if (offset == 0 && chunk == sector_bytes) {
			// Whole-sector write.
			if (!write(sector, 1, in)) {
				delete[] sector_buffer;
				return false;
			}
		} else {
			// Read-modify-write.
			if (!read(sector, 1, sector_buffer)) {
				delete[] sector_buffer;
				return false;
			}

			memcpy(sector_buffer + offset, in, chunk);

			if (!write(sector, 1, sector_buffer)) {
				delete[] sector_buffer;
				return false;
			}
		}

		current_address += chunk;
		in += chunk;
		remaining -= chunk;
	}

	delete[] sector_buffer;
	return true;
}

}
