#pragma once
#include <stdint.h>

typedef enum {
  SYSCALL_DBG,

  SYSCALL_FOPEN,
  SYSCALL_FCLOSE,
  SYSCALL_FREAD,
  SYSCALL_FWRITE,
  SYSCALL_FLSEEK,
  SYSCALL_FSTAT,

  MAX_SYSCALL
} syscall_t;

extern uint64_t syscall(syscall_t syscall_number, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e);

uint64_t syscall_test();

uint64_t syscall_handler(syscall_t syscall_number, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e);