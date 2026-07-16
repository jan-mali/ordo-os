#pragma once

#include <cstdint>
namespace Interrupts {

void enable();
void disable();
uint64_t store_and_disable();
void restore(uint64_t flags);
void init();

}
