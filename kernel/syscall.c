#include "syscall.h"
#include "printf.h"
#include "kerror.h"

uint64_t syscall_handler(syscall_t syscall_number, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
  switch (syscall_number) {
    case SYSCALL_DBG:
      return 0;
      break;
    case SYSCALL_FOPEN:
    case SYSCALL_FCLOSE:
    case SYSCALL_FREAD:
    case SYSCALL_FWRITE:
    case SYSCALL_FLSEEK:
    case SYSCALL_FSTAT:
      return KERROR_INVALID_SYSCALL;
      break;
    default:
      return KERROR_INVALID_SYSCALL;
  }
}

uint64_t syscall_test() {
  return syscall(SYSCALL_DBG, 1,2,3,4,5);
}