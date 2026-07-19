COMMON_WARNINGS := \
    -Wall \
    -Wextra \
    -Wpedantic

COMMON_FLAGS := \
    -ffreestanding \
    -frtti \
    -fno-exceptions \
    -fno-stack-protector \
    -fno-builtin \
    -fno-tree-vectorize \
    -fno-tree-slp-vectorize \
    -fno-omit-frame-pointer \
    -fno-optimize-sibling-calls \
    -mno-sse \
    -mno-sse2 \
    -mno-sse3 \
    -mno-ssse3 \
    -mno-sse4 \
    -mno-sse4.1 \
    -mno-sse4.2 \
    -mno-avx \
    -mno-red-zone \
    -march=x86-64 \
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

LIBS := -lgcc -lsupc++
