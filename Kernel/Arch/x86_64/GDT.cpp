#include "Logger/Logger.hpp"
#include <Arch/x86_64/GDT.hpp>
#include <cstdint>

namespace GDT {

GDTR gdtr;
GDTEntry entries[7];
TSS tss;

extern "C" void gdt_load(GDTR*);
extern "C" void tss_load();

void init() {
    entries[0] = { // Null descriptor
        .normal = {
            0, 0, 0, 0, 0, 0, 0
        }
    };

    entries[1] = { // Kernel Code
        .normal = {
            .limit0 = 0xFFFF,
            .base0 = 0,
            .base16 = 0,
            .access_byte = 0x9A,
            .limit16 = 0xF,
            .flags = 0b1010,
            .base24 = 0
        }
    };

    entries[2] = { // Kernel Data
        .normal = {
            .limit0 = 0xFFFF,
            .base0 = 0,
            .base16 = 0,
            .access_byte = 0x92,
            .limit16 = 0xF,
            .flags = 0b1100,
            .base24 = 0
        }
    };

    entries[3] = { // User Code
        .normal = {
            .limit0 = 0xFFFF,
            .base0 = 0,
            .base16 = 0,
            .access_byte = 0xFA,
            .limit16 = 0xF,
            .flags = 0b1010,
            .base24 = 0
        }
    };

    entries[4] = { // User Data
        .normal = {
            .limit0 = 0xFFFF,
            .base0 = 0,
            .base16 = 0,
            .access_byte = 0xF2,
            .limit16 = 0xF,
            .flags = 0b1100,
            .base24 = 0
        }
    };

    tss.iopb = sizeof(TSS);

    uint64_t tss_base = reinterpret_cast<uint64_t>(&tss);
    uint16_t tss_base0 = tss_base & 0xFFFF;
    uint8_t tss_base16 = (tss_base >> 16) & 0xFF;
    uint8_t tss_base24 = (tss_base >> 24) & 0xFF;
    uint32_t tss_base32 = (tss_base >> 32) & 0xFFFFFFFF;

    static_assert(sizeof(TSS) - 1 <= 0xFFFF, "TSS' doesn't fit in the lower 16 bits of the limit field");
    uint16_t tss_limit0 = sizeof(TSS) - 1;

    entries[5] = { // TSS Low
        .normal = {
            .limit0 = tss_limit0,
            .base0 = tss_base0,
            .base16 = tss_base16,
            .access_byte = 0x89,
            .limit16 = 0,
            .flags = 0b0000,
            .base24 = tss_base24,
        }
    };

    entries[6] = { // TSS High
        .tss_high = {
            .base32 = tss_base32
        }
    };

    // Linear Address
    gdtr.offset = reinterpret_cast<uint64_t>(&entries);
    gdtr.size = sizeof(entries) - 1;

    // TODO: Change this to something actually usable
    tss.rsp0 = 0xDEADC0DE;

    gdt_load(&gdtr);
    tss_load();
    Logger::log_info("GDT & TSS are initialized.\n");
}

}