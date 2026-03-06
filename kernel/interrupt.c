#include "interrupt.h"

void interrupt_handler(uint64_t interrupt_number, uint64_t error_code) {
    
}

void sti() {
    __asm__ volatile ("sti");
}

void cli() {
    __asm__ volatile ("cli");
}