#!/usr/bin/env bash

if [ ! -f "kernel8.img" ]; then
    echo "Error: kernel8.img not found! Run 'make' first."
    exit 1
fi

QEMU_CMD=(
    qemu-system-aarch64
    -M raspi4b
    -m 2G
    -smp 4
    -kernel kernel8.img
    -serial stdio
    # Mount the raw FAT32 block image instead of the virtual folder
    -drive file=sd.img,format=raw,if=sd
)

if [[ "$1" == "-d" || "$1" == "--debug" ]]; then
    echo "Starting QEMU in GDB debug mode..."
    QEMU_CMD+=(-s -S)
fi

"${QEMU_CMD[@]}"