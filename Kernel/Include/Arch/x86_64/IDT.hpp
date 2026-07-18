#pragma once

#include <Arch/x86_64/Registers.hpp>
#include <cstdint>

namespace Interrupts::IDT {

constexpr uint8_t FIRST_DYNAMIC_VECTOR = 0x81;
constexpr uint8_t LAST_DYNAMIC_VECTOR = 0xFE;

constexpr uint8_t DYNAMIC_VECTOR_COUNT = LAST_DYNAMIC_VECTOR - FIRST_DYNAMIC_VECTOR + 1;

uint8_t allocate_dynamic_vector();
void set_dynamic_handler(uint8_t vector, void (*handler)(Registers*));

extern void (*dynamic_handlers[DYNAMIC_VECTOR_COUNT])(Registers*);

}
