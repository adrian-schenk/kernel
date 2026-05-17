#include "spinlock.h"

uint8_t spinlock_init() {
  return 0;
}

void lock(uint8_t *lock) {
  while (__atomic_exchange_n(lock, 1, __ATOMIC_ACQUIRE|__ATOMIC_HLE_ACQUIRE)) 
    asm volatile("pause");
}

void unlock(uint8_t *lock) {
  __atomic_store_n(lock, 0, __ATOMIC_RELEASE|__ATOMIC_HLE_RELEASE);
}