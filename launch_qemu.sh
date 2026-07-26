#!/usr/bin/env bash

if [ ! -f "kernel8.img" ]; then
    echo "Error: kernel8.img not found! Run 'make' first."
    exit 1
fi

QEMU_CMD=(
    qemu-system-aarch64 -M raspi4b -m 2G -kernel kernel8.img -serial stdio
)

"${QEMU_CMD[@]}"