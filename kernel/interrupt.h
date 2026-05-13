#pragma once
#include <stdint.h>

typedef void(*interrupt_handler_t)(uint64_t interrupt_number, uint64_t error_code);

extern interrupt_handler_t interrupt_handlers[256];

void interrupts_setup();

void set_interrupt_handler(uint64_t interrupt_number, interrupt_handler_t);

void interrupt_handler(uint64_t interrupt_number, uint64_t error_code);

void sti();
void cli();