This is an _**experimental**_ kernel for the x86 architecture

Feel free to play around with it. 

## Features

- 64-bit long mode
- VESA graphics with 8-bit font
- x87/SSE capabilities
- GDT with both kernel and user space, as well as per-core TSS
- Exchangeable interrupt handlers
- SMP and multithreading
- Task scheduler and context switches
- PCI device detection and enumeration
- AHCI setup
- APIC and IOAPIC setup
- APIC-Timer calibration
- Basic keyboard driver for user input
- Multiple privilege-levels (r0 and r3)
- Basic syscalls
- Multiple blockdevs: ramfs and ata
- ext4 file-system (WIP)

## Planned features

- Higher half relocation
- Processes
- Malloc for userspace
- Basic shell
- Window creation
- ext4 write-operations
- ext4 mkfs
- RNG
- NIC/Ethernet Controller + network stack
- Single booting from ata drive

## Getting started

It is expected that you already have a C-compiler for the x86 architecture as well as python and make installed.
For emulating you need Qemu.

Run `make rebuild-disk1` to create the disk image which will contain the file system

Create a filesystem inside the disk image using `mkfs.ext4 disk1.img`

(Optional): You can mount the image and create some files/folders using `mount -o loop disk1.img /mnt/xy`

Build the kernel using `make`

Start the qemu using `make run` or `make run-gdb` for debugging sessions

##  Technical Info

This project uses the AT&T/GNU assembly syntax.

### Project Structure

This project uses `Makefile` to build and write the kernel to the final image.

Usually, each subdirectory has its own `Makefile` to build the current directory which the kernel might depend on.

Most boot/early stage assembly code is located in `arc/x86/boot`, although some is located in `kernel/` (e.g interrupt service routines, context switch logic)

Device driver code is located in `dev/` (just keyboard driver for now)

File system logic and generic structs are located in `fs` and the respective subdirectories (`ext4`)

### Boot protocol

The kernel is designed to be booted directly from a floppy disk. All entry-level code is self-written and no external bootloaders are used.

The execution starts at address `0x7c00` in real-mode in `boot.S`, which sets up some basic C-runtime as well as loading the remaining kernel from the floppy disk. Note that the `dap` structure to load `kernelcore.S` and the kernel itself are modified by `scripts/build.py`.
This is simply to avoid padding the kernel with the old signatures, that are no longer used.
After successfully loading the kernel, `boot.S` jumps to `kernelcore.S`.

`kernelcore.S` sets up VESA graphics and makes the protected mode switch. it then sets up a identity-mapped page table and switches to 64-bit long mode.

`kernecore_64.S` is just a "pitstop" to enable x87/SSE and then jump to `kernel_main`.

### Memory Layout

| Region | Start | End | Size | Purpose |
| --- | --- | --- | --- | --- |
| Bootloader | `0x7C00` | `0x7DFF` | `0x200` (512 B) | BIOS boot sector load area (`BOOTBLOCK_START`) |
| Initial stack top | `0xFFF0` | `0xFFF0` | `0x1000` | Initial stack top (`INTERRUPT_STACK_TOP`) |
| BSP PM setup | `0x1000` |  | base address | Protected-mode setup and long-mode transition (`PM_START`) |
| BIOS Memory Map | `0x5000` | `0x7000` | `0x2000` | BIOS Memory Map Information from INT 15h E820 |
| TSS region | `0x7000` | `0x8000` | base address | TSS structure base (`TSS_REGION`) |
| AP boot info | `0x8000` | `0x10000` | base address | AP bootstrap info pointer area (`AP_BOOT_INFO_ADDR`) |
| Trampoline | `0x10000` | `0x10000` | trampoline | AP core startup code|
| 64-bit kernel entry | `0x80000` |  | kernel size | Kernel entry/load start (`KERNEL_START`) |
| Page Table Section | `0x100000` | `0x17FFFF` | `7FFFF` (0.5 MiB) | Page Table Allocation Area |
| Physical Table Section | `0x180000` | `0x19FFFF` | `0x7FFFF` (0.5 MiB) | Physical Page Allocation Area |
| Kernel heap (`kmalloc`) | `0x200000` | `0x2FFFFF` | `0x100000` (1 MiB) | Early dynamic allocation (`KMALLOC_START`, `KMALLOC_LENGTH`) |
| Main memory | `0x300000` | up | N/A | General memory start (`MAIN_MEMORY_START`) |