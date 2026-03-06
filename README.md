based on dthain/basekernel

Memory Layout
0x7c00-0x7e00 -> Bootloader
0xfff0 -> Stack
0x1000 -> BSP PM Setup + LM transition
0x5000 -> TSS
0x6000 -> AP_BOOT_INFO_ADDR
0x8000 -> AP Trampoline
0x10000 -> 64 Bit Kernel