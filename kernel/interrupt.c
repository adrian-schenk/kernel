#include "interrupt.h"
#include "printf.h"
#include "apic.h"
#include "io.h"
#include "timer.h"
#include "cpu.h"

void unknown_interrupt(uint64_t interrupt_number, uint64_t error_code) {
    printf("received interrupt %d with error: %d on cpu: %d\n", interrupt_number, error_code, this_cpu(cpu_id));
    for(;;);
}

void exception_handler(uint64_t exception_number, uint64_t error_code) {}

interrupt_handler_t interrupt_handlers[256] = { unknown_interrupt };
interrupt_handler_t interrupt_handlers_ap[256] = { unknown_interrupt };

void interrupt_handler(uint64_t interrupt_number, uint64_t error_code) {

    this_cpu(interrupt_handlers)[interrupt_number](interrupt_number, error_code);
    
    apic_write(EOI_REGISTER, 0);
}

void interrupts_setup() {
    for (int i = 0; i < 256; i++) {
        interrupt_handlers[i] = unknown_interrupt;
        interrupt_handlers_ap[i] = unknown_interrupt;
    }
}

void set_interrupt_handler(interrupt_handler_t *handlers, uint64_t interrupt_number, interrupt_handler_t handler) {
    handlers[interrupt_number] = handler;
}

void sti() {
    __asm__ volatile ("sti");
}

void cli() {
    __asm__ volatile ("cli");
}