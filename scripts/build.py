import mmap
import math

read_start = 1

files = [
    "arch/x86/boot/kernelcore.bin",
    "arch/x86/boot/trampoline.bin",
    "kernel/kernel.bin",
]

with open("arch/x86/boot/boot.bin", "r+b") as boot:
    with mmap.mmap(boot.fileno(), length=0, access=mmap.ACCESS_READ) as mm:
        position = mm.find(b"\x76\x98\x34\x12")  # signature to find the DAP
        print(f"Signature found at position: {position}")
        if position == -1:
            raise Exception("Signature not found in boot.bin")
        position += 4

with open("arch/x86/boot/boot.bin", "r+b") as boot:
    for i in range(len(files)):
        with open(files[i], "rb") as f:
            f.seek(0, 2)  # Move to the end of the file
            size = f.tell()  # Get the size of the file
            print(f"Size of {files[i]}: {size} bytes")
            sectors = math.ceil(size / 512)  # Calculate the number of sectors
            print(f"Sectors needed for {files[i]}: {sectors}")

            boot.seek(position + (i * 16) + 2)  # Move to the sector count position
            boot.write(sectors.to_bytes(2, byteorder='little'))  # Write the sector count
            print(f"Written sector count for {files[i]} at position: {position + (i * 16) + 2}")
            boot.seek(4, 1)
            boot.write((read_start).to_bytes(4, byteorder='little'))  # Write the starting sector
            print(f"Written starting sector for {files[i]} at position: {position + (i * 16) + 6}")
            read_start += sectors # Update the starting sector for the next file
    