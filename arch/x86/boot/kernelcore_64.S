[BITS 32]
section .text
global start

start:
    ; Enable Physical Address Extension (PAE) by setting bit 5 in CR4
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    ; Set up the paging structures at physical address 0x300000
    mov edi, 0x300000       ; EDI = base address for PML4

    ; Write PML4 entry
    mov eax, 0x30100F       ; PDPT address + present + writable + user
    stosd
    mov ecx, 0x03FF         ; Fill the rest with zeros
    xor eax, eax
    rep stosd

    ; Write PDPT entry
    mov eax, 0x30200F       ; PDT address + present + writable + user
    stosd
    mov ecx, 0x03FF
    xor eax, eax
    rep stosd

    ; Write Page Directory with 2 MiB pages
    mov ecx, 0x200          ; 512 entries for 2 MiB = 1 GiB
    mov esi, 0x008B         ; 2 MiB page | writable | present

map_loop:
    mov eax, esi            ; Lower 32-bits of PDPTE: flags + base addr
    stosd
    xor eax, eax            ; Upper 32-bits of PDPTE: 0
    stosd
    add esi, 0x200000       ; Next 2 MiB
    sub ecx, 1
    jnz map_loop

    ; Load the address of the PML4 into CR3
    mov eax, 0x300000
    mov cr3, eax

    ; Enable Long Mode via EFER MSR (0xC0000080)
    mov ecx, 0xC0000080     ; EFER MSR
    rdmsr
    or eax, 0x00000100      ; Enable Long Mode (LME bit)
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 0x80000000      ; Set PG bit (bit 31)
    mov cr0, eax

    ; Far jump to 64-bit code
    jmp 0x30:long_mode_start

; --- GDT (for use before far jump) ---
; Should contain a 64-bit code segment at selector 0x30
; You must define this GDT somewhere and load it beforehand with LGDT.

[BITS 64]
long_mode_start:
    ; We are now in 64-bit long mode!
    ; You can put your 64-bit kernel entry here
    jmp $

