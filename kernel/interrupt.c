#include "interrupt.h"
#include "printf.h"
#include "apic.h"
#include "io.h"

void interrupt_handler(uint64_t interrupt_number, uint64_t error_code) {
    uint8_t scancode = inb(0x60);
    apic_write(EOI_REGISTER, 0);
}

void sti() {
    __asm__ volatile ("sti");
}

void cli() {
    __asm__ volatile ("cli");
}