BUILD_DIR := Build

C_STANDARD   := c17
CXX_STANDARD := c++20

INCLUDE_DIRS := \
    Kernel/Include \
    Kernel/Arch/x86_64/Include

INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))
