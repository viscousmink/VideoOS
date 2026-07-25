#!/usr/bin/env python3
import os
import struct

def create_test_bmp(filepath, width, height):
    # BMP rows must be padded to a multiple of 4 bytes
    row_padding = (4 - (width * 3) % 4) % 4
    row_size = (width * 3) + row_padding
    pixel_data_size = row_size * height
    file_size = 54 + pixel_data_size

    # 14-byte BMP Header
    # 'BM' (2), Size (4), Reserved1 (2), Reserved2 (2), Offset (4)
    bmp_header = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, 54)

    # 40-byte DIB Header (BITMAPINFOHEADER)
    # Size (4), Width (4), Height (4), Planes (2), BPP (2), Compression (4)
    # ImageSize (4), XpixelsPerM (4), YpixelsPerM (4), ColorsUsed (4), ColorsImportant (4)
    dib_header = struct.pack('<IiiHHIIIIII', 
                             40, width, height, 1, 24, 0, 
                             pixel_data_size, 2835, 2835, 0, 0)

    # Ensure the target directory exists
    os.makedirs(os.path.dirname(filepath), exist_ok=True)

    with open(filepath, 'wb') as f:
        f.write(bmp_header)
        f.write(dib_header)

        # Generate Pixel Data (Written Bottom-to-Top, Left-to-Right)
        # 24-bit BMP uses BGR order, not RGB
        for y in range(height):
            for x in range(width):
                # Create a gradient: Blue varies by X, Red varies by Y
                b = x % 256
                g = 0       # Keep green empty for contrast
                r = y % 256
                
                f.write(struct.pack('<BBB', b, g, r))
            
            # Add padding at the end of the row
            for _ in range(row_padding):
                f.write(b'\x00')

    print(f"Successfully baked {width}x{height} 24-bit BMP to: {filepath}")

if __name__ == "__main__":
    # Point this directly to the virtual FAT folder QEMU mounts
    target_path = os.path.join("./", "image.bmp")
    create_test_bmp(target_path, 256, 256)