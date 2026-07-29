#!/bin/bash
set -e

IMG=disk.img
SIZE=512M

BOOT_LIMINE=Boot/Limine
KERNEL=Build/Kernel/kernel.elf

WORK=Build/mount
ROOTFS=$WORK/rootfs

INITRAMFS=Initramfs
INITRAMFS_TAR=Build/initramfs.tar
ROOT=Base

INIT_BINARY=Build/Userspace/init/init

rm -f "$IMG"
truncate -s "$SIZE" "$IMG"

parted -s "$IMG" \
    mklabel gpt \
    mkpart ESP fat32 1MiB 65MiB \
    set 1 esp on \
    mkpart ROOT ext2 65MiB 100%

LOOP=$(sudo losetup --find --show --partscan "$IMG")

sudo mkfs.fat -F32 ${LOOP}p1
sudo mkfs.ext2 -L ROOTFS -O none ${LOOP}p2

mkdir -p "$WORK/esp"
mkdir -p "$ROOTFS"

sudo mount ${LOOP}p1 "$WORK/esp"
sudo mount ${LOOP}p2 "$ROOTFS"

sudo mkdir -p "$WORK/esp/EFI/BOOT"
sudo mkdir -p "$WORK/esp/boot"

sudo cp "$BOOT_LIMINE/BOOTX64.EFI" \
    "$WORK/esp/EFI/BOOT/BOOTX64.EFI"

sudo cp "$KERNEL" \
    "$WORK/esp/boot/kernel.elf"

sudo cp Boot/limine.conf \
    "$WORK/esp/boot/"

sudo cp -a "$ROOT"/. "$ROOTFS"/

# Init
sudo cp -a "$INIT_BINARY" "$ROOTFS"/bin/init

echo "Creating initramfs..."

rm -f "$INITRAMFS_TAR"

tar \
    --format=ustar \
    -C "$INITRAMFS" \
    -cf "$INITRAMFS_TAR" \
    .

echo "Copying initramfs..."

sudo cp "$INITRAMFS_TAR" \
    "$WORK/esp/boot/initramfs.tar"

sudo umount "$ROOTFS"
sudo umount "$WORK/esp"

sudo losetup -d "$LOOP"

echo "Built $IMG"