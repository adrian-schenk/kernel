
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

    # Save BIOS boot drive
    mov %dl, boot_drive

    # Setup stack
    mov $INTERRUPT_STACK_SEGMENT, %ax
    mov %ax, %ss
    mov $INTERRUPT_STACK_OFFSET, %sp

    # Print message
    mov $msg, %si
    call print_string

load:
    # DS must point to the DAPs
    mov %cs, %ax
    mov %ax, %ds

    # Load kernelcore
    mov $dap_kernelcore, %si
    mov $0x42, %ah
    mov boot_drive, %dl
    int $0x13
    jc disk_error

    # Load kernel in chunks
    # BP = total sectors remaining
    movw dap_kernel+2, %bp

kernel_load_loop:
    testw %bp, %bp
    jz kernel_load_done

    # AX = min(remaining sectors, 32)
    movw %bp, %ax
    cmpw $32, %ax
    jbe kernel_chunk_ready
    movw $32, %ax

kernel_chunk_ready:
    # Set DAP sector count for this read
    movw %ax, dap_kernel+2
    pushw %ax

    # BIOS extended read
    movw $dap_kernel, %si
    movb $0x42, %ah
    movb boot_drive, %dl
    int $0x13
    jc disk_error_kernel

    # Restore chunk size
    popw %ax

    # Remaining sectors -= chunk size
    subw %ax, %bp

    # Destination segment += chunk * 32 paragraphs
    movw %ax, %bx
    shlw $5, %bx
    addw %bx, dap_kernel+6

    # Starting LBA (low 32 bits) += chunk size
    movzwl %ax, %eax
    addl %eax, dap_kernel+8
    adcl $0, dap_kernel+12

    jmp kernel_load_loop

kernel_load_done:
    # Kernel loaded at physical 0x80000
    ljmp $PM_SEGMENT, $PM_OFFSET

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
# Boot drive number saved from DL at entry
#
boot_drive:
    .byte 0x00

#
# Disk Address Packets
#

dap_start:
    .long 0x12349876

dap_kernelcore:
    .byte 0x10
    .byte 0x00
    .word 0x0000        # Sectors patched by build.py
    .word 0x1000        # Offset
    .word 0x0000        # Segment: physical 0x10000
    .long 0x00000000    # LBA low
    .long 0x00000000    # LBA high

dap_trampoline:
    .byte 0x10
    .byte 0x00
    .word 0x0000        # Sectors patched by build.py
    .word 0x0000        # Offset
    .word 0x1000        # Segment: physical 0x10000
    .long 0x00000000    # LBA low
    .long 0x00000000    # LBA high

dap_kernel:
    .byte 0x10
    .byte 0x00
    .word 0x0000        # Total sectors patched by build.py
    .word 0x0000        # Offset
    .word 0x8000        # Segment: physical 0x80000
    .long 0x00000000    # LBA low
    .long 0x00000000    # LBA high

.org 510
.word 0xAA55