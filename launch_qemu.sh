#!/usr/bin/env bash

# Exit immediately if kernel8.img doesn't exist
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
)

# Pass -d or --debug to freeze QEMU for GDB attachment on port 1234
if [[ "$1" == "-d" || "$1" == "--debug" ]]; then
    echo "Starting QEMU in GDB debug mode (waiting for target remote :1234)..."
    QEMU_CMD+=(-s -S)
else
    echo "Starting QEMU..."
fi

# Run QEMU
"${QEMU_CMD[@]}"