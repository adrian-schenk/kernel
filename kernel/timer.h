#pragma once
#include <stdint.h>
#include "cpu.h"

#define TIMER_DIVIDE_1 0b1
#define TIMER_DIVIDE_2 0x00
#define TIMER_DIVIDE_4 0b0001 
#define TIMER_DIVIDE_8 0b0010
#define TIMER_DIVIDE_16 0b0011
#define TIMER_DIVIDE_32 0b1000
#define TIMER_DIVIDE_64 0b1001
#define TIMER_DIVIDE_128 0b1010

#define TIMER_ONE_SHOT 0 << 16
#define TIMER_PERIODIC 1 << 17
#define TIMER_DISABLED 1 << 16

#define TIMER_INTERRUPT 32

void timer_setup();

unsigned long long get_time();
int get_ms_counter();

void rtc_interrupt(uint64_t interrupt_number, uint64_t error_code);
void apic_timer_setup_interrupt(uint64_t interrupt_number, uint64_t error_code);

void timer_setup_ap(struct cpu_local *cpu_local);