#include "timer.h"
#include "apic.h"
#include "io.h"
#include "interrupt.h"
#include "printf.h"
#include "cpu.h"
#include "boot_info.h"
#include "globals.h"

volatile int rtc_ticks = 0;

const int rtc_calibration_ticks = 256;
static const uint32_t apic_calibration_initial_count = 0xFFFFFFFFu;
static uint8_t rtc_prev_reg_b = 0;

long long apic_speed = 0;

static void rtc_enable_periodic_interrupts(void)
{
  set_interrupt_handler(this_cpu(interrupt_handlers), 40, rtc_interrupt);

  uint32_t low =
      0x28         // vector
      | (0 << 8)   // fixed delivery
      | (0 << 11)  // physical destination
      | (0 << 13)  // active high
      | (0 << 15)  // edge trigger
      | (0 << 16); // unmasked

  ioapic_write(0x20, low);
  ioapic_write(0x21, 0 << 24);

  outb(0x70, 0x8B);
  rtc_prev_reg_b = inb(0x71);
  outb(0x70, 0x8B);
  outb(0x71, rtc_prev_reg_b | 0x40);
}

static void rtc_disable_periodic_interrupts(void)
{
  outb(0x70, 0x8B);
  outb(0x71, rtc_prev_reg_b);
  outb(0x70, 0x0C);
  inb(0x71);
}

static uint64_t apic_measure_rtc_window_counts(int calibration_ticks)
{
  int start_tick = rtc_ticks + 1;
  while (rtc_ticks < start_tick)
    ;

  apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_ONE_SHOT);
  apic_write(TIMER_INITIAL_COUNT_REGISTER, apic_calibration_initial_count);

  uint32_t start_count = apic_read(TIMER_CURRENT_COUNT_REGISTER);
  int end_tick = rtc_ticks + calibration_ticks;
  while (rtc_ticks < end_tick)
    ;
  uint32_t end_count = apic_read(TIMER_CURRENT_COUNT_REGISTER);

  apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_DISABLED);

  return (uint32_t)(start_count - end_count);
}

// will only run on bs core
void rtc_interrupt(uint64_t interrupt_number, uint64_t error_code)
{
  rtc_ticks++;
  outb(0x70, 0x0C); // select register C
  inb(0x71);        // just throw away contents
}

// will run on all cores
void apic_timer_setup_interrupt(uint64_t interrupt_number, uint64_t error_code)
{
  this_cpu(apic_ticks)++;
}

// will run on all cores
void apic_interrupt(uint64_t interrupt_number, uint64_t error_code)
{
  this_cpu(apic_time) += this_cpu(ms_counter);
  this_cpu(last_schedule) += this_cpu(ms_counter);
  if (this_cpu(last_schedule) > this_cpu(ms_counter) * 2)
  {
    this_cpu(last_schedule) = 0;
    scheduler_tick(this_cpu(scheduler));
  }
}

void timer_setup()
{

  // enable RTC interrupts for apic timer calibration
  cli();
  rtc_enable_periodic_interrupts();
  sti();

  apic_write(TIMER_DIVIDE_CONFIGURATION_REGISTER, TIMER_DIVIDE_1); // divide by 1

  uint64_t calibration_counts = apic_measure_rtc_window_counts(rtc_calibration_ticks);
  rtc_disable_periodic_interrupts();

  if (calibration_counts > 0)
  {
    apic_speed = (calibration_counts * 1024ULL) / rtc_calibration_ticks;
    kprintf("Detected APIC Timer speed: %d\n", apic_speed);
  }
  else
  {
    kprintf("APIC Timer could not be initialized\n");
  }

  this_cpu(ms_counter) = apic_speed / 1000;

  set_interrupt_handler(this_cpu(interrupt_handlers), 32, apic_interrupt);
  apic_write(TIMER_INITIAL_COUNT_REGISTER, this_cpu(ms_counter));   // set counter
  apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_PERIODIC); // enable apic timer
}

void timer_setup_ap(struct cpu_local *cpu_local)
{

  apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_DISABLED); // disable apic timer (if enabled)
  apic_write(TIMER_DIVIDE_CONFIGURATION_REGISTER, TIMER_DIVIDE_1);  // divide by 1

  boot_info->ap_startup_done = 1; // boot up other cores

  while (timer_phase < 1)
    ;

  apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_ONE_SHOT);
  apic_write(TIMER_INITIAL_COUNT_REGISTER, apic_calibration_initial_count);

  while (timer_phase < 2)
    ;

  uint32_t remaining_counts = apic_read(TIMER_CURRENT_COUNT_REGISTER);
  uint64_t calibration_counts = apic_calibration_initial_count - remaining_counts;

  apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_DISABLED); // disable apic timer

  // set regular interrupt handlers
  cpu_local->interrupt_handlers = &interrupt_handlers;

  long long apic_speed = calibration_counts * (1000 / timer_calib_ms);
  cpu_local->ms_counter = apic_speed / 1000;
  cpu_local->apic_time = 0;

  apic_write(TIMER_INITIAL_COUNT_REGISTER, cpu_local->ms_counter);  // set counter (for 1 ms)
  apic_write(LVT_TIMER_REGISTER, TIMER_INTERRUPT | TIMER_PERIODIC); // enable periodic timer (to enable sleep)
}

unsigned long long get_time()
{
  return this_cpu(apic_time);
}

int get_ms_counter()
{
  return this_cpu(ms_counter);
}