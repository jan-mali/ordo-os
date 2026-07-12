COMMON_WARNINGS := \
    -Wall \
    -Wextra \
    -Wpedantic

COMMON_FLAGS := \
    -ffreestanding \
    -fno-exceptions \
    -fno-rtti \
    -fno-stack-protector \
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

CFLAGS   += -O2 -g
CXXFLAGS += -O2 -g

LDFLAGS := \
    -nostdlib \
    -T Boot/linker.ld

LIBS := -lgcc
