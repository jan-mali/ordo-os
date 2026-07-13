#pragma once

namespace Interrupts {

static void enable() {
    asm volatile ("sti");
}

static void disable() {
    asm volatile ("cli");
}

}
