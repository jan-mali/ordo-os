#pragma once

#include <Drivers/Device.hpp>
#include <cstdint>

namespace Drivers {

class BlockDevice : public Device {
public:
	virtual bool read(uint64_t lba, uint64_t count, uint8_t* buffer) = 0;
	virtual bool write(uint64_t lba, uint64_t count, uint8_t* buffer) = 0;
	virtual uint64_t sector_size() = 0;
	virtual uint64_t sector_count() = 0;

	bool read_bytes(uint64_t address, uint64_t bytes, uint8_t* buffer);
	bool write_bytes(uint64_t address, uint64_t bytes, uint8_t* buffer);

	virtual ~BlockDevice() = default;

private:
};

}
