#include <Logger/Logger.hpp>
#include <Arch/x86_64/Interrupts.hpp>
#include <Arch/x86_64/IO.hpp>
#include <cstdint>

extern "C" {
    void init_isr_and_lidt();
}

namespace Interrupts {

void enable() {
    asm volatile ("sti");
}

void disable() {
    asm volatile ("cli");
}

extern "C" uint8_t idt[];

void init() {
    init_isr_and_lidt();

    Logger::log_info("IDT is initalized.\n");

}

}