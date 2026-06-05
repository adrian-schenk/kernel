#include "gdt.h"
#include "tss.h"
#include "kmalloc.h"
#include "memlayout.h"

void set_gdt_entry(struct gdt_entry* gdt, int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[num].base_low    = (base & 0xFFFF);     // unused in long mode
    gdt[num].base_middle = (base >> 16) & 0xFF; // unused in long mode
    gdt[num].base_high   = (base >> 24) & 0xFF; // unused in long mode

    gdt[num].limit   = (limit & 0xFFFF);        // unused in long mode
    gdt[num].granularity = (limit >> 16) & 0x0F; // unused in long mode
    gdt[num].granularity |= (flags & 0xF0);
    gdt[num].access      = access;
}

void set_tss_entry(struct gdt_entry* gdt, int num, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
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

void gdt_setup(struct gdt_entry* gdt, uint16_t gdt_entries) {
    struct gdt_ptr gdt_init;
    gdt_init.limit = (gdt_entries * sizeof(struct gdt_entry)) - 1;
    gdt_init.base = (uint64_t)gdt;

    int size = 0;
    set_gdt_entry(gdt, 0, 0, 0, 0, 0); // null descriptor
    set_gdt_entry(gdt, 1, 0, 0xFFFFFFFF, GDT_A_PRESENT | GDT_A_PRIVL_0 | GDT_A_CODE_DATA | GDT_A_EXECUTABLE | GDT_A_RW, GDT_F_GRANULARITY | GDT_F_64BIT); // 64-bit kernel code segment
    set_gdt_entry(gdt, 2, 0, 0xFFFFFFFF, GDT_A_PRESENT | GDT_A_PRIVL_0 | GDT_A_CODE_DATA | GDT_A_RW, GDT_F_GRANULARITY); // 64-bit kernel data segment
    set_gdt_entry(gdt, 3, 0, 0xFFFFFFFF, GDT_A_PRESENT | GDT_A_PRIVL_3 | GDT_A_CODE_DATA | GDT_A_EXECUTABLE | GDT_A_RW, GDT_F_GRANULARITY | GDT_F_64BIT); // 64-bit user code segment
    set_gdt_entry(gdt, 4, 0, 0xFFFFFFFF, GDT_A_PRESENT | GDT_A_PRIVL_3 | GDT_A_CODE_DATA | GDT_A_RW, GDT_F_GRANULARITY); // 64-bit user data segment

    struct tss *gdt_tss = kmalloc_early(sizeof(struct tss));
    void *interrupt_stack = kmalloc_early(2048);
    gdt_tss->rsp0 = (void*)((uint8_t*)interrupt_stack + 2048);
    gdt_tss->io_map_base = sizeof(struct tss);

    set_tss_entry(gdt, 5, (uint64_t)gdt_tss, sizeof(struct tss) - 1, 0x89, 0);

    gdt_load(gdt_init);

    return;
}

void gdt_load(struct gdt_ptr gdt_init) {
    asm volatile ("lgdt %0" : : "m" (gdt_init));
    asm volatile ("mov $0x28, %ax\n" // Load TSS, so selector value is 5*8=40 or 0x28
                  "ltr %ax\n");
    
    asm volatile (
        "mov $0x10, %%ax\n" // Data segment selector (index 2)
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