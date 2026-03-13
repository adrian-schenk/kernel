#include "idt.h"

struct idt_init idt_init = {};
struct idt idt = {};

void _idt_set_gate(uint8_t num, uint64_t offset, uint16_t selector, uint8_t ist, uint8_t type_attr) {
    idt.gates[num].offset_low = offset & 0xFFFF;
    idt.gates[num].selector = selector;
    idt.gates[num].ist = ist;
    idt.gates[num].type_attr = type_attr;
    idt.gates[num].offset_mid = (offset >> 16) & 0xFFFF;
    idt.gates[num].offset_high = (offset >> 32) & 0xFFFFFFFF;
    idt.gates[num].zero = 0;
}

void idt_setup() {

    idt_init.limit = sizeof(struct idt) - 1;
    idt_init.base = (uint64_t)&idt;

    for (int i = 0; i < 48; i++) {
        idt_set_gate(i, 0, 0x08, 0, 0x8E);
    }

    idt_set_gate(47, 0, 0x08, 0, 0x8E);

    idt_load();
}

void idt_load() {
    __asm__ volatile ("lidt %0" : : "m"(idt_init));
}