#include "gdt.h"

void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt_[num].base_low    = (base & 0xFFFF);
    gdt_[num].base_middle = (base >> 16) & 0xFF;
    gdt_[num].base_high   = (base >> 24) & 0xFF;

    gdt_[num].limit_low   = (limit & 0xFFFF);
    gdt_[num].granularity = (limit >> 16) & 0x0F;
    gdt_[num].granularity |= (granularity & 0xF0);
    gdt_[num].access      = access;
}

void gdt_setup() {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt_;

    set_gdt_entry(0, 0, 0, 0, 0);

    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    return;
    //gdt_flush((uint32_t)&gp);
}