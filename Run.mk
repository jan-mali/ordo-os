run: image
	qemu-system-x86_64 \
		-machine q35,accel=kvm \
		-cpu host \
		-m 2G \
		-drive if=none,id=disk,file=disk.img,format=raw \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/x64/OVMF_CODE.4m.fd \
		-drive if=pflash,format=raw,file=OVMF_VARS.4m.fd \
		-device virtio-blk-pci,drive=disk \
		-bios /usr/share/OVMF/OVMF_CODE.fd \
		-serial stdio

debug: image
	qemu-system-x86_64 \
		-machine q35,accel=kvm \
		-cpu host \
		-m 2G \
		-drive if=none,id=disk,file=disk.img,format=raw \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/x64/OVMF_CODE.4m.fd \
		-drive if=pflash,format=raw,file=OVMF_VARS.4m.fd \
		-device virtio-blk-pci,drive=disk \
		-bios /usr/share/OVMF/OVMF_CODE.fd \
		-serial stdio \
		-s -S
