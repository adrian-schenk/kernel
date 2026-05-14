#include "timer.h"
#include "apic.h"
#include "io.h"
#include "interrupt.h"
#include "printf.h"

volatile int rtc_ticks = 0;
volatile int apic_ticks = 0;

unsigned long long apic_time;

const int rtc_calibration_ticks = 256;

long long apic_speed = 0;
int ms_counter;

void rtc_interrupt(uint64_t interrupt_number, uint64_t error_code) {
    rtc_ticks++;
    outb(0x70, 0x0C);	// select register C
    inb(0x71);		// just throw away contents
}

void apic_timer_setup_interrupt(uint64_t interrupt_number, uint64_t error_code) {
    apic_ticks++;
}

void apic_interrupt(uint64_t interrupt_number, uint64_t error_code) {
    apic_time += ms_counter;
}

void timer_setup() {

    // enable RTC interrupts for apic timer calibration
    cli();
    set_interrupt_handler(40, rtc_interrupt);
    uint32_t low =
        0x28          // vector
        | (0 << 8)    // fixed delivery
        | (0 << 11)   // physical destination
        | (0 << 13)   // active high
        | (0 << 15)   // edge trigger
        | (0 << 16);  // unmasked

    ioapic_write(0x20, low);
    uint32_t high = (0 << 24);
    ioapic_write(0x21, high);

    outb(0x70, 0x8B); // select register B and disable NMI for RTC interrupts
    char prev=inb(0x71);	// read the current value of register B
    outb(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
    outb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
    sti();

    // apic setup
    set_interrupt_handler(32, apic_timer_setup_interrupt);
    apic_write(TIMER_DIVIDE_CONFIGURATION_REGISTER, TIMER_DIVIDE_1); // divide by 2
    
    while (rtc_ticks < 2);
    
    apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_PERIODIC); // enable periodic timer
    apic_write(TIMER_INITIAL_COUNT_REGISTER, 10240 << 2); // set counter

    while (rtc_ticks < rtc_calibration_ticks + 2);

    apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_DISABLED); // disable apic timer
    outb(0x70, 0x0B); // disable rtc interrupts

    float seconds = rtc_ticks / 1024.0f;

    if (seconds > 0) {
        apic_speed = (uint64_t)(((10240 << 2) * apic_ticks / 2) / seconds);
        printf("Detected APIC Timer speed: %d\n", apic_speed);
    } else {
        printf("APIC Timer could not be initialized\n");
    }

    ms_counter = apic_speed / 1000 * 2;
    
    set_interrupt_handler(32, apic_interrupt);
    apic_write(TIMER_INITIAL_COUNT_REGISTER, ms_counter); // set counter
    apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_PERIODIC); // enable apic timer
}

unsigned long long get_time() {
    return apic_time;
}

int get_ms_counter() {
    return ms_counter;
}