#include "apic.h"
#include "io.h"
#include "cpu.h"

void apic_setup() {
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);   
}