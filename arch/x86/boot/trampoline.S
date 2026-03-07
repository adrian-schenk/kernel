#include "memlayout.h"

.code16
.text
.global _start

/*
    Will be loaded to 0x10000
*/

_start:
    cli
    cld

    mov	%cs, %ax		# Set up the extra segment
    mov	%ax, %es		# with the data segment
    mov	%ax, %ds        # Set up the data segment
    mov	%ax, %ss        # Set up the stack segment
    mov	%ax, %gs        # Set up the stack segment
    mov	%ax, %fs        # Set up the stack segment

    mov $AP_STACK_TOP, %ax
    mov %ax, %sp        # Set up the stack pointer (bs core stack is at 0xfff0 right now)

    # enable A20 line
    inb $0x92, %al      
    orb $2, %al
    outb %al, $0x92
    mov $0xFF, %ax
    
    # Load GDT
    lgdt (gdt_init-_start)

    # Enter protected mode
    mov %cr0, %eax
    or $1, %eax
    mov %eax, %cr0

    mov	$(2*8), %ax		# selector two is flat 4GB data data
    mov	%ax, %ds		# set data, extra, and stack segments to selector two
    mov	%ax, %es
    mov	%ax, %ss

    xor %ax, %ax
    mov %ax, %fs
    mov %ax, %gs

    # Far jump to flush prefetch queue and switch to protected mode
    ljmpl $(1*8), $pm_start

.code32
pm_start:
    
    # Update TSS for long mode
    mov gdt64 + (3*8), %ax  # TSS selector
    add $0x680000, %ax # point to next tss selector in tss space
    
    call enable_paging
    lgdt gdt64_init

    mov $0x18, %ax
    ltr %ax

    mov $AP_BOOT_INFO_ADDR, %eax
    mov 32(%eax), %eax
    
    push $0x08
    push %eax

    lretl

enable_paging:
    mov $AP_BOOT_INFO_ADDR, %eax
    mov (%eax), %eax
  	mov %eax, %cr3
    
  	mov %cr4, %eax
  	or $1 << 5, %eax
  	mov %eax, %cr4
    
  	mov $0xC0000080, %ecx
  	rdmsr
  	or $((1 << 8) | (1 << 11)), %eax
  	wrmsr
    
  	xor %eax, %eax
  	mov %cr0, %eax
  	or $((1 << 31) | (1 << 0)), %eax
  	mov %eax, %cr0
    jmp 1f
    1:
    
  	ret  

.section .data
.align 16
.global gdt
gdt:
    .word	0,0,0,0				            # seg 0 - null
    .word	0xffff, 0x0000, 0x9a00, 0x00cf	# seg 1 - kernel flat 4GB code
    .word	0xffff, 0x0000, 0x9200, 0x00cf	# seg 2 - kernel flat 4GB data
    .word	0xffff, 0x0000, 0xfa00, 0x00cf	# seg 3 - user flat 4GB code
    .word	0xffff, 0x0000, 0xf200, 0x00cf	# seg 4 - user flat 4GB data

gdt_init:
    .word	gdt_init-gdt
    .long	gdt

.align 16
.global gdt64
gdt64:
    .quad 0x0000000000000000    # null descriptor
    .quad 0x00A09A0000000000    # 64-bit kernel code segment
    .quad 0x00A0920000000000    # 64-bit kernel data segment
    .quad 0x0000890070000068    # TSS (0x7000)
    .quad 0x0000000000000000

gdt64_end:

gdt64_init:
    .word gdt64_end - gdt64  # limit (size)
    .long gdt64, 0x0000

ljmplabel:
    .long 0,0