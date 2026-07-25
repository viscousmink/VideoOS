#!/usr/bin/env bash
set -e

# Configuration
IMAGE_NAME="sd.img"
IMAGE_SIZE_MB=64
TEST_FILE="video.mp4"

echo "==> Creating ${IMAGE_SIZE_MB}MB blank image..."
dd if=/dev/zero of="${IMAGE_NAME}" bs=1M count=${IMAGE_SIZE_MB} status=progress

echo "==> Formatting ${IMAGE_NAME} as FAT32..."
mkfs.vfat -F 32 "${IMAGE_NAME}"

echo "==> Creating dummy '${TEST_FILE}' file..."
# Creates a non-empty 1MB dummy file so Circle reads actual data blocks
dd if=/dev/urandom of="tmp_${TEST_FILE}" bs=1M count=1 status=none

echo "==> Copying '${TEST_FILE}' onto SD image root..."
# Ensure mtools is installed
if ! command -v mcopy &> /dev/null; then
    echo "Error: 'mtools' is not installed. Run 'sudo apt install mtools' first."
    rm -f "tmp_${TEST_FILE}"
    exit 1
fi

mcopy -i "${IMAGE_NAME}" "tmp_${TEST_FILE}" "::${TEST_FILE}"
rm -f "tmp_${TEST_FILE}"

echo "==> Verifying SD image contents:"
mdir -i "${IMAGE_NAME}" ::

echo "==> Done! '${IMAGE_NAME}' is ready for QEMU."