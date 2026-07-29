BUILD_DIR := ../Build/Kernel

C_STANDARD   := c17
CXX_STANDARD := c++20

INCLUDE_DIRS := \
    Include

INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))
