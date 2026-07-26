#!/usr/bin/env python3
import os
import struct
import sys

def convert_raw_to_bmp(raw_filename, bmp_filename, width, height):
    if not os.path.exists(raw_filename):
        print(f"Error: File '{raw_filename}' not found.")
        sys.exit(1)

    expected_raw_size = width * height * 3
    actual_file_size = os.path.getsize(raw_filename)

    print(f"Reading '{raw_filename}' ({actual_file_size} bytes)...")
    if actual_file_size != expected_raw_size:
        print(f"[Warning] File size ({actual_file_size}) does not match expected size ({expected_raw_size}) for {width}x{height} RGB24!")

    with open(raw_filename, "rb") as f:
        raw_bytes = f.read()

    # BMP rows must be padded to a multiple of 4 bytes
    row_padding_len = (4 - (width * 3) % 4) % 4
    row_padding = b'\x00' * row_padding_len
    
    pixel_data_size = (width * 3 + row_padding_len) * height
    file_size = 54 + pixel_data_size  # 54 bytes for header + pixel payload

    # 1. BMP File Header (14 bytes)
    # Format: 'BM' (2s), FileSize (I), Reserved1 (H), Reserved2 (H), PixelDataOffset (I)
    file_header = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, 54)

    # 2. DIB Header / BITMAPINFOHEADER (40 bytes)
    # Format: HeaderSize (I), Width (i), Height (i), Planes (H), BitsPerPixel (H),
    #         Compression (I), ImageSize (I), XpixelsPerM (i), YpixelsPerM (i), ColorsUsed (I), ColorsImportant (I)
    dib_header = struct.pack('<IiiHHIIiiII', 
                             40,             # DIB header size
                             width,          # Image width
                             height,         # Image height (positive = bottom-to-top)
                             1,              # Color planes
                             24,             # Bits per pixel (RGB24)
                             0,              # Compression (0 = BI_RGB uncompressed)
                             pixel_data_size,# Image data size
                             2835,           # Horizontal resolution (72 DPI in pixels/meter)
                             2835,           # Vertical resolution (72 DPI in pixels/meter)
                             0,              # Number of colors in palette
                             0)              # Important colors

    with open(bmp_filename, "wb") as f:
        # Write headers
        f.write(file_header)
        f.write(dib_header)

        # Write pixels: BMPs store rows bottom-to-top, and pixels in BGR order
        for y in range(height - 1, -1, -1):
            row_start = y * width * 3
            row_pixels = raw_bytes[row_start : row_start + (width * 3)]

            # Convert RGB -> BGR for BMP compatibility
            bgr_row = bytearray()
            for x in range(0, len(row_pixels), 3):
                r = row_pixels[x]
                g = row_pixels[x + 1]
                b = row_pixels[x + 2]
                bgr_row.extend([b, g, r])

            f.write(bgr_row)
            f.write(row_padding)

    print(f"Successfully wrote '{bmp_filename}' ({file_size} bytes).")

if __name__ == "__main__":
    # Match these to your image dimensions
    WIDTH = 256
    HEIGHT = 256
    RAW_FILE = "image.raw"
    BMP_FILE = "output.bmp"

    convert_raw_to_bmp(RAW_FILE, BMP_FILE, WIDTH, HEIGHT)