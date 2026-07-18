#pragma once

#include <Drivers/Device.hpp>
#include <cstdint>

class BlockDevice : public Device {
public:
	virtual bool read(uint64_t lba, uint64_t count, uint8_t* buffer) = 0;
	virtual bool write(uint64_t lba, uint64_t count, uint8_t* buffer) = 0;

private:
};
