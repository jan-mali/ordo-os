IMAGE := disk.img

image: $(KERNEL)
	@bash Scripts/build_image.sh
