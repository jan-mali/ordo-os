BUILD_DIR := ../../Build/Userspace/init

C_STANDARD   := c17
CXX_STANDARD := c++20

INCLUDE_DIRS := \
    ../LibC/Include

INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))
