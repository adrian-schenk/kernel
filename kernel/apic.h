#pragma once
#include <stdint.h>
#include "sdt.h"

#define SPURIOUS_INTERRUPT_VECTOR_REGISTER 0xF0
#define LOCAL_DESTINATION_REGISTER 0xD0
#define DESTINATION_FORMAT_REGISTER 0xE0
#define EOI_REGISTER 0xB0

void apic_setup();
void apic_write(uint16_t reg, uint32_t value);
uint32_t apic_read(uint16_t reg);

uint32_t ioapic_read(uint32_t reg);
void ioapic_write(uint32_t reg, uint32_t value);