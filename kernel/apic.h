#pragma once
#include <stdint.h>

void apic_setup();

uint32_t ioapic_read(uint32_t reg);
void ioapic_write(uint32_t reg, uint32_t value);