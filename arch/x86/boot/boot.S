#include <memlayout.h>

.global _start

.code16
.text

ljmp $BOOTBLOCK_SEGMENT, $_start

_start:
    cli
    cld

    # Set segments correctly
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es

    # Setup stack
    mov $INTERRUPT_STACK_SEGMENT, %ax
    mov %ax, %ss
    mov $INTERRUPT_STACK_OFFSET, %sp

    # Print message
    mov $msg, %si
    call print_string

load:
    # DS must point to dap
    mov %cs, %ax
    mov %ax, %ds
    mov $dap_kernelcore, %si

    mov $0x42, %ah
    mov $0x80, %dl

    int $0x13
    jc disk_error

    mov $dap_kernel, %si

    mov $0x42, %ah
    mov $0x80, %dl

    int $0x13
    jc disk_error_kernel

    mov $dap_trampoline, %si

    mov $0x42, %ah
    mov $0x80, %dl

    int $0x13
    jc disk_error_trampoline

    ljmp $PM_SEGMENT, $PM_OFFSET # jump to kernelcore at 0x10000 physical

success:
    mov $ok_msg, %si
    call print_string
    hlt

disk_error:
    mov $err_msg, %si
    call print_string
    hlt

disk_error_kernel:
    mov $err_msg_kernel, %si
    call print_string
    hlt

disk_error_trampoline:
    mov $err_msg_trampoline, %si
    call print_string
    hlt

#
# print_string
#
print_string:
    .loop:
        lodsb
        test %al, %al
        jz .done
        mov $0x0E, %ah
        int $0x10
        jmp .loop
    .done:
        ret

#
# Strings
#
msg:
    .asciz "Loading Kernelcore...\r\n"

ok_msg:
    .asciz "Disk read OK\r\n"

err_msg:
    .asciz "Disk read error (Kernelcore)!\r\n"

err_msg_kernel:
    .asciz "Disk read error (Kernel)!\r\n"

err_msg_trampoline:
    .asciz "Disk read error (Trampoline)!\r\n"

#
# Disk Address Packet
# Loads kernel to physical 0x10000
#
dap_start:
    .long 0x12349876
dap_kernelcore:
    .byte 0x10          # size (16 bytes)
    .byte 0x00          # reserved

    .word 0x0000        # sectors to read (will be changed by scripts/build.py)

    .word 0x1000        # offset
    .word 0x0000        # segment → 0x1000 physical

    .long 0x00000000    # LBA low (will be changed by scripts/build.py)
    .long 0x00000000    # LBA high

dap_trampoline:
    .byte 0x10          # size (16 bytes)
    .byte 0x00          # reserved

    .word 0x0000        # sectors to read (will be changed by scripts/build.py)

    .word 0x8000        # offset
    .word 0x0000        # segment → 0x0000 physical

    .long 0x00000000    # LBA low (will be changed by scripts/build.py)
    .long 0x00000000    # LBA high

dap_kernel:
    .byte 0x10          # size (16 bytes)
    .byte 0x00          # reserved

    .word 0x0000        # sectors to read (will be changed by scripts/build.py)

    .word 0x0000        # offset
    .word 0x1000        # segment → 0x10000 physical

    .long 0x00000000    # LBA low (will be changed by scripts/build.py)
    .long 0x00000000    # LBA high

.org 510
.word 0xAA55