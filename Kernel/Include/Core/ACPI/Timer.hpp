#pragma once

#include <Core/ACPI/Interrupts.hpp>

namespace ACPI::Timer {

constexpr uint32_t LAPIC_TIMER_HZ = 100;

void init();
void finish_calibration();

extern uint64_t monotonic_time_ns;

}
