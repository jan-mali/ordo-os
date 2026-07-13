COMMON_WARNINGS := \
    -Wall \
    -Wextra \
    -Wpedantic

COMMON_FLAGS := \
    -ffreestanding \
    -fno-exceptions \
    -fno-rtti \
    -fno-stack-protector \
    -march=x86-64 \
    -fno-builtin \
    -mno-sse \
    -mno-sse2 \
    -mno-sse3 \
    -mno-ssse3 \
    -mno-sse4 \
    -mno-sse4.1 \
    -mno-sse4.2 \
    -mno-avx \
    -mno-red-zone \
    -fno-tree-vectorize \
    -fno-tree-slp-vectorize \
    -mcmodel=kernel

CFLAGS := \
    $(COMMON_WARNINGS) \
    $(COMMON_FLAGS) \
    -std=$(C_STANDARD) \
    -MMD -MP \
    $(INCLUDE_FLAGS)

CXXFLAGS := \
    $(COMMON_WARNINGS) \
    $(COMMON_FLAGS) \
    -std=$(CXX_STANDARD) \
    -MMD -MP \
    $(INCLUDE_FLAGS)

ASFLAGS := \
    -f elf64

CFLAGS   += -O2 -g
CXXFLAGS += -O2 -g

LDFLAGS := \
    -nostdlib \
    -T Boot/linker.ld

LIBS := -lgcc
