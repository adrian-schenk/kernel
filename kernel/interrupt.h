#pragma once
#include <stdint.h>

void interrupt_handler(uint64_t interrupt_number, uint64_t error_code);

void sti();
void cli();