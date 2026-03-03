.code16
.text
.global _start

_start:
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

    # Far jump to flush prefetch queue and switch to protected mode
    ljmpl $(1*8), $pm_start

.code32
pm_start:
    
    hlt

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
