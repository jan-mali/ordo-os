CPP_SOURCES := $(shell find Kernel Lib -type f -name '*.cpp')
C_SOURCES   := $(shell find Kernel Lib -type f -name '*.c')

OBJECTS := \
    $(CPP_SOURCES:%.cpp=$(BUILD_DIR)/%.o) \
    $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)

DEPENDENCIES := $(OBJECTS:.o=.d)

KERNEL := $(BUILD_DIR)/kernel.elf
