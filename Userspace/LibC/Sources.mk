CPP_SOURCES := $(shell find . -type f -name '*.cpp')
C_SOURCES   := $(shell find . -type f -name '*.c')
ASM_SOURCES := $(shell find . -type f -name '*.asm')

OBJECTS := \
    $(CPP_SOURCES:%.cpp=$(BUILD_DIR)/%.o) \
    $(C_SOURCES:%.c=$(BUILD_DIR)/%.o) \
    $(ASM_SOURCES:%.asm=$(BUILD_DIR)/%.o)

DEPENDENCIES := $(OBJECTS:.o=.d)

LIBC := $(BUILD_DIR)/LibC.a
