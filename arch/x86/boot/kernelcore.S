#include <memlayout.h>

.code16
.text
.global _start
_start:
    jmp _realstart

_realstart:

    mov	$0x4f02, %ax
    movw $0x4118, %bx
    int	$0x10
    
    mov	%cs, %ax		# Set up the extra segment
    mov	%ax, %es		# with the data segment
    mov	%ax, %ds        # Set up the data segment
    mov	%ax, %ss        # Set up the stack segment
    mov	%ax, %gs        # Set up the stack segment
    mov	%ax, %fs        # Set up the stack segment

    # enable A20 line
    inb $0x92, %al      
    orb $2, %al
    outb %al, $0x92

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

    mov	$INTERRUPT_STACK_TOP, %sp    # set up initial C stack
    mov	$INTERRUPT_STACK_TOP, %bp    # set up initial C stack
    
    # Far jump to flush prefetch queue and switch to protected mode
    ljmpl $(1*8), $pm_start

.code32
pm_start:
    #setup long mode

    .globl set_ap_boot_info
    call set_ap_boot_info

    call setup_page_tables 
    call enable_paging

    # were in long mode now

    # setup gdt for long mode
    lgdt gdt64_init
    
	ljmpl $0x08, $0x10000

.global setup_page_tables
.global setup_page_loop

setup_page_tables:
  	mov $page_table_l3, %eax
  	or $0b11, %eax
  	mov %eax, (page_table_l4)

  	mov $page_table_l2, %eax
  	or $0b11, %eax
  	mov %eax, (page_table_l3)

  	mov $0, %ecx

.setup_page_loop:
  	mov $0x200000, %eax

  	mul %ecx
  	or $0b10000011, %eax
  	mov %eax, page_table_l2(, %ecx, 8)

  	inc %ecx
  	cmp $512, %ecx
  	jne .setup_page_loop

  	ret

enable_paging:
  	mov $page_table_l4, %eax
  	mov %eax, %cr3

  	mov %cr4, %eax
  	or $1 << 5, %eax
  	mov %eax, %cr4

  	mov $0xC0000080, %ecx
  	rdmsr
  	or $1 << 8, %eax
  	wrmsr

  	xor %eax, %eax
  	mov %cr0, %eax
  	or $((1 << 31) | (1 << 0)), %eax
  	mov %eax, %cr0

  	ret

.align 16
.global gdt
gdt:
    .word	0,0,0,0				            # seg 0 - null
    .word	0xffff, 0x0000, 0x9a00, 0x00cf	# seg 1 - kernel flat 4GB code
    .word	0xffff, 0x0000, 0x9200, 0x00cf	# seg 2 - kernel flat 4GB data
    .word	0xffff, 0x0000, 0xfa00, 0x00cf	# seg 3 - user flat 4GB code
    .word	0xffff, 0x0000, 0xf200, 0x00cf	# seg 4 - user flat 4GB data
	
# This is the initializer for the global descriptor table.
# It simply tells us the size and location of the table.

gdt_init:
    .word	gdt_init-gdt
    .long	gdt

.align 16
.global gdt64
gdt64:
    .quad 0x0000000000000000    # null descriptor
    .quad 0x00A09A0000000000    # 64-bit kernel code segment
    .quad 0x00A0920000000000    # 64-bit kernel data segment

gdt64_end:

# 64-bit GDTR descriptor (10 bytes)
gdt64_init:
    .word gdt64_end - gdt64  # limit (size - 1)
    .long gdt64, 0x0000

.bss
.align 4096
.global page_table_l4
page_table_l4:
  .space 4096
page_table_l3:
  .space 4096
page_table_l2:
  .space 4096