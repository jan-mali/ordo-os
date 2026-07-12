include Config.mk
include Toolchain.mk
include Flags.mk
include Sources.mk
include Build.mk
include Image.mk
include Run.mk

.PHONY: all clean rebuild image run debug

all: $(KERNEL)
	@echo
	@echo "Kernel built successfully"

clean:
	rm -rf Build disk.img

rebuild: clean
	$(MAKE)
