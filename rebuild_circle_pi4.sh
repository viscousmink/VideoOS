#!/usr/bin/env bash
set -e

CIRCLE_DIR="tools/circle"
RECONFIG=0

# Parse command line flags (e.g. ./build.sh -c)
while getopts "c" opt; do
  case ${opt} in
    c )
      RECONFIG=1
      ;;
    \? )
      echo "Usage: $0 [-c]"
      echo "  -c  Force deep clean and reconfigure Circle"
      exit 1
      ;;
  esac
done

if [ ! -d "${CIRCLE_DIR}" ]; then
    echo "Error: Circle directory not found at '${CIRCLE_DIR}'."
    exit 1
fi

echo "==> Cleaning local project..."
make clean || true

cd "${CIRCLE_DIR}"

if [ "${RECONFIG}" -eq 1 ]; then
    echo "==> Cleaning Circle tree..."
    ./makeall clean || true
    rm -f Config.mk Config.old
fi

echo "==> Configuring Circle for Pi 4 (64-bit + QEMU with 32 bpp)..."
./configure -r 4 -p "aarch64-none-elf-" -d AARCH=64 --qemu --multicore -f

echo "==> Rebuilding Circle libraries (32 bpp)..."
./makeall -j20

echo "==> Returning to project root..."
cd - > /dev/null

echo "==> Compiling application kernel..."
make -j20 DEPTH=32

echo "==> Complete! kernel8.img is rebuilt for Raspberry Pi 4."