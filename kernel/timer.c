#include "timer.h"
#include "apic.h"

void timer_setup() {
    apic_write(TIMER_DIVIDE_CONFIGURATION_REGISTER, 0x00); // divide by 2
    apic_write(LVT_TIMER_REGISTER, 0x20 | (1 << 17)); // periodic
    apic_write(TIMER_INITIAL_COUNT_REGISTER, 0x10000); // set counter to 0
}