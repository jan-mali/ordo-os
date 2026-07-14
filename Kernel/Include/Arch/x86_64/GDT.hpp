#pragma once

#include <cstdint>

namespace GDT {

struct __attribute__((packed)) GDTR {
	uint16_t size;
	uint64_t offset;
};

struct __attribute__((packed)) GDTEntryNormal {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base16;
	uint8_t access_byte;
	uint8_t limit16 : 4;
	uint8_t flags : 4;
	uint8_t base24;
};

struct __attribute__((packed)) GDTEntryTSSHigh {
	uint32_t base32;
	uint32_t reserved;
};

union GDTEntry {
	GDTEntryNormal normal;
	GDTEntryTSSHigh tss_high;
};

static_assert(sizeof(GDTEntryNormal) == 8);
static_assert(sizeof(GDTEntryTSSHigh) == 8);
static_assert(sizeof(GDTEntry) == 8);
static_assert(sizeof(GDTR) == 10);

extern "C" GDTR gdtr;
extern GDTEntry entries[];

struct __attribute__((packed)) TSS {
	uint32_t reserved0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved1;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iopb;
};

extern TSS tss;

void init();

}
