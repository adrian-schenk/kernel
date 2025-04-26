#include "interrupt.h"
#include "pic.h"
#include "printf.h"

static int_handler interrupt_handlers[48];

void unknown_interrupt(int i, int code) { return; }

void interrupt_init() {
  for (int i = 0; i < 16; i++) {
    pic_disable(i);
    pic_acknowledge(i);
  }

  for (int i = 0; i < 48; i++) {
    interrupt_register(i, unknown_interrupt);
  }

  interrupt_unblock();
  pic_enable(1);
  pic_enable(12);
}

char interrupt_register(int i, int_handler handler) {
  interrupt_handlers[i] = handler;
}

void interrupt_handler(int i, int code) {
  printf("%d", i);
  interrupt_handlers[i](i, code);
  pic_acknowledge(i - 32);
}

void interrupt_block() { asm("cli"); }

void interrupt_unblock() { asm("sti"); }
