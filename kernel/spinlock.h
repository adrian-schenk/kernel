#pragma once
#include <stdint.h>

uint8_t spinlock_init();

void lock();
void unlock();