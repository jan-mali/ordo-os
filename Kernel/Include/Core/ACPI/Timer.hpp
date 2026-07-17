#pragma once

#include <Core/ACPI/Interrupts.hpp>

namespace ACPI::Timer {

constexpr uint32_t LAPIC_TIMER_HZ = 50;

void init();
void finish_calibration();

}
