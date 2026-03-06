## Memory Layout

| Region | Start | End | Size | Purpose |
| --- | --- | --- | --- | --- |
| Bootloader | `0x7C00` | `0x7DFF` | `0x200` (512 B) | BIOS boot sector load area (`BOOTBLOCK_START`) |
| Interrupt stack top | `0xFFF0` | `0xFFF0` | 1 address | Initial stack top (`INTERRUPT_STACK_TOP`) |
| BSP PM setup | `0x1000` | `0x1000` | entry address | Protected-mode setup and long-mode transition (`PM_START`) |
| TSS region | `0x7000` | `0x7000` | base address | TSS structure base (`TSS_REGION`) |
| AP boot info / trampoline | `0x8000` | `0x8000` | base address | AP bootstrap info pointer and trampoline load area (`AP_BOOT_INFO_ADDR`) |
| 64-bit kernel entry | `0x10000` | `0x10000` | entry address | Kernel entry/load start (`KERNEL_START`) |
| Kernel heap (`kmalloc`) | `0x100000` | `0x1FFFFF` | `0x100000` (1 MiB) | Early dynamic allocation (`KMALLOC_START`, `KMALLOC_LENGTH`) |
| Main memory | `0x200000` | up | N/A | General memory start (`MAIN_MEMORY_START`) |
