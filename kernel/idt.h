#pragma once
#include <stdint.h>

extern uint64_t isr_stub_table[];

#define idt_set_gate(idt, num, offset, selector, ist, type_attr) \
    _idt_set_gate(idt, num, (uint64_t)(isr_stub_table[num]), selector, ist, type_attr)

struct idt_gate {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt {
    struct idt_gate gates[256];
} __attribute__((packed));

struct idt_init {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_setup(struct idt* idt);
void idt_load(struct idt_init idt_init);
void _idt_set_gate(struct idt* idt, uint8_t num, uint64_t offset, uint16_t selector, uint8_t ist, uint8_t type_attr);