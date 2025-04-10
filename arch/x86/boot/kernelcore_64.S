.code32
.section .data
.align 4096
pml4_table:
    .quad pdpt_table + 3               # Add 3 directly for flags instead of bitwise OR

.align 4096
pdpt_table:
    .quad pd_table + 3

.align 4096
pd_table:
    .quad pt_table + 3

.align 4096
pt_table:
    .rept 512
    .quad 0x83                         # Identity-map entries with flags directly if you have constants
    .endr

.align 16
gdt64:
    .quad 0x0000000000000000           # NULL descriptor
    .quad 0x00af9a000000ffff           # 64-bit code segment descriptor
    .quad 0x00af92000000ffff           # 64-bit data segment descriptor

gdt_descriptor:
    .word 23                           # Limit (size of GDT - 1)
    .long gdt64                        # Lower 32-bits of GDT address
    .long 0x0                          # Upper 32-bits of GDT address (hardcoded to 0 for compatibility)

.section .text
.global lm_jmp
lm_jmp:
    # Clear the PG bit in CR0 to temporarily disable paging
    mov %cr0, %eax
    and $0x7FFFFFFF, %eax              # Clear bit 31
    mov %eax, %cr0

    # Load the GDT (32-bit compatible load)
    movl $gdt_descriptor, %eax
    lgdt (%eax)

    # Enable PAE in CR4
    movl %cr4, %eax
    bts $5, %eax                       # Set PAE (Physical Address Extension) bit
    movl %eax, %cr4

    # Enable Long Mode in EFER MSR
    movl $0xC0000080, %ecx             # EFER MSR
    rdmsr                              # Read EFER into EDX:EAX
    bts $8, %eax                       # Set LME (Long Mode Enable) bit
    wrmsr                              # Write back to EFER MSR

    # Load the PML4 table into CR3 to enable paging structures
    movl $pml4_table, %eax
    movl %eax, %cr3

    # Enable Paging in CR0 by setting the PG bit
    movl %cr0, %eax
    bts $31, %eax                      # Set PG (Paging) bit to enable paging
    movl %eax, %cr0

    # Perform a far jump to 64-bit code segment to enter Long Mode
    ljmp $0x08, $long_mode_entry       # Jump to 64-bit code segment

.align 16
.code64
long_mode_entry:
    # Continue with 64-bit code (now in 64-bit mode)
    mov $0x10, %ax                     # Load 64-bit data segment selector
    mov %ax, %ds                       # Set DS
    mov %ax, %es                       # Set ES
    mov %ax, %ss                       # Set SS

    # 64-bit code (example)
    mov $0x1, %rax                     # Example: load immediate into RAX
    mov $0x2, %rbx                     # Example: load immediate into RBX
    add %rbx, %rax                     # RAX = RAX + RBX

    jmp .

.halt:
    hlt
    jmp .halt                          # Infinite loop to halt execution
