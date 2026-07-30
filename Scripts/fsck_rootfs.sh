#!/bin/bash
set -e

IMG=disk.img

LOOP=$(sudo losetup --find --show --partscan "$IMG")

cleanup() {
    sudo losetup -d "$LOOP"
}

trap cleanup EXIT

echo "Checking ROOTFS partition..."

sudo e2fsck -f -v -n ${LOOP}p2

echo "Filesystem check complete."