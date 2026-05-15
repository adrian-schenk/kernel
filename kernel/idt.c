#include "idt.h"

void _idt_set_gate(struct idt* idt, uint8_t num, uint64_t offset, uint16_t selector, uint8_t ist, uint8_t type_attr) {
    idt->gates[num].offset_low = offset & 0xFFFF;
    idt->gates[num].selector = selector;
    idt->gates[num].ist = ist;
    idt->gates[num].type_attr = type_attr;
    idt->gates[num].offset_mid = (offset >> 16) & 0xFFFF;
    idt->gates[num].offset_high = (offset >> 32) & 0xFFFFFFFF;
    idt->gates[num].zero = 0;
}

void idt_setup(struct idt* idt) {
    struct idt_init idt_init;
    idt_init.limit = sizeof(struct idt) - 1;
    idt_init.base = (uint64_t)idt;

    for (int i = 0; i < 48; i++) {
        idt_set_gate(idt, i, 0, 0x08, 0, 0x8E);
    }

    idt_set_gate(idt, 47, 0, 0x08, 0, 0x8E);

    idt_load(idt_init);
}

void idt_load(struct idt_init idt_init) {
    __asm__ volatile ("lidt %0" : : "m"(idt_init));
}