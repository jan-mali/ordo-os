IMAGE := disk.img

.PHONY: image

image: all
	@bash Scripts/build_image.sh
