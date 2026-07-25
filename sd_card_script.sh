#!/usr/bin/env bash
set -e

# 1. Create a 1GB sparse blank image
dd if=/dev/zero of=sd.img bs=1M count=0 seek=1024

# 2. Partition it with an MBR table (Fat32 starting at 1MB offset)
parted -s sd.img mktable msdos mkpart primary fat32 1MiB 100%

# 3. Format Partition 1 as FAT32
mformat -i "sd.img@@1048576" -F ::

# 4. Copy the image into the root directory
mcopy -i "sd.img@@1048576" image.bmp ::IMAGE.BMP