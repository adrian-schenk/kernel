#include "sleep.h"
#include "timer.h"
#include "interrupt.h"
#include "printf.h"

void sleep(int millis) {
  unsigned long long time = get_time();
  int ms_counter = get_ms_counter();
  while (get_time() < time + millis * ms_counter);
}

void usleep(long long micros) {
  unsigned long long time = get_time();
}