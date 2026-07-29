.PHONY: all clean rebuild

all:
	make -C Kernel/
	make -C Userspace/LibC/
	make -C Userspace/init/

clean:
	rm -rf Build disk.img

rebuild: clean
	$(MAKE)

include Image.mk
include Run.mk