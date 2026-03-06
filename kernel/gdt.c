#include "gdt.h"
#include "tss.h"
#include "memlayout.h"

struct gdt_entry gdt[1 + SEGMENT_DESCRIPTORS + (TSS_DESCRIPTORS * 2)];
struct gdt_ptr gdt_init;

void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[num].base_low    = (base & 0xFFFF);     // unused in long mode
    gdt[num].base_middle = (base >> 16) & 0xFF; // unused in long mode
    gdt[num].base_high   = (base >> 24) & 0xFF; // unused in long mode

    gdt[num].limit   = (limit & 0xFFFF);        // unused in long mode
    gdt[num].granularity = (limit >> 16) & 0x0F; // unused in long mode
    gdt[num].granularity |= (flags & 0xF0);
    gdt[num].access      = access;
}

void set_tss_entry(int num, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    struct gdt_entry_tss* tss_entry = (struct gdt_entry_tss*) &gdt[num];

    tss_entry->limit       = limit & 0xFFFF;
    tss_entry->base_low    = base & 0xFFFF;
    tss_entry->base_middle = (base >> 16) & 0xFF;
    tss_entry->access      = access;
    tss_entry->granularity = ((limit >> 16) & 0x0F) | (flags & 0xF0);
    tss_entry->base_high   = (base >> 24) & 0xFF;

    tss_entry->base_upper  = (base >> 32) & 0xFFFFFFFF;
    tss_entry->reserved    = 0;
}

void gdt_setup() {
    //gdt_init.limit = sizeof(gdt);
    //gdt_init.base = (uint64_t)&gdt;
//
    //set_gdt_entry(0, 0, 0, 0, 0); // null descriptor
    //set_gdt_entry(1, 0, 0xFFFFFFFF, GDT_A_PRESENT | //GDT_A_PRIVL_0 | GDT_A_CODE_DATA | GDT_A_EXECUTABLE | //GDT_A_RW, GDT_F_GRANULARITY | GDT_F_64BIT); // 64-bit //kernel code segment
    //set_gdt_entry(2, 0, 0xFFFFFFFF, GDT_A_PRESENT | //GDT_A_PRIVL_0 | GDT_A_CODE_DATA | GDT_A_RW, //GDT_F_GRANULARITY | GDT_F_64BIT); // 64-bit kernel data //segment
//
    //tss.rsp0 = INTERRUPT_STACK_TOP;
    //tss.io_map_base = sizeof(struct tss);
//
    //set_tss_entry(3, &tss, sizeof(struct tss), 0x89, 0);

    gdt_init.limit = 0x28;
    gdt_init.base = 0x1110;

    gdt_load();

    return;
}

void gdt_load() {
    asm volatile ("lgdt %0" : : "m" (gdt_init));
    asm volatile ("mov $18, %ax\n" // Load TSS or //////////ex 3, so selector value is 3*8=24 or 0x18)
                "ltr %ax\n");

    asm volatile (
        "mov $0x10, %%ax\n" // Data segment selector (index 
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "push $0x08\n" // Code segment selector (index 1)
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        :
        :
        : "memory", "rax"
    );

    
}