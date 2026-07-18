#pragma once

#include <cstdint>

namespace PCI::ConfigurationSpace {

uint8_t read8(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
uint16_t read16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
uint32_t read32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);

void write8(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint8_t value);
void write16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint16_t value);
void write32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value);

};
