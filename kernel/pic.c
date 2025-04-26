#include "pic.h"
#include "printf.h"
#include <stdint.h>

#define PIC_ICW1 0x11
#define PIC_ICW4_MASTER 0x01
#define PIC_ICW4_SLAVE 0x05
#define PIC_ACK_SPECIFIC 0x60

#define MASTER_DATA 0x21
#define MASTER_CMD 0x20

#define SLAVE_DATA 0xa1
#define SLAVE_CMD 0xa0

static uint8_t pic_control[2] = {0x20, 0xa0};
static uint8_t pic_data[2] = {0x21, 0xa1};

void pic_init(int pic0base, int pic1base) {
  outb(PIC_ICW1, MASTER_CMD);
  outb(pic0base, MASTER_DATA);
  outb(1 << 2, MASTER_DATA);
  outb(PIC_ICW4_MASTER, MASTER_DATA);
  outb(~(1 << 2), MASTER_DATA);

  outb(PIC_ICW1, SLAVE_CMD);
  outb(pic1base, SLAVE_DATA);
  outb(2, SLAVE_DATA);
  outb(PIC_ICW4_SLAVE, SLAVE_DATA);
  outb(~0, SLAVE_DATA);

  printf("pic: ready\n");
}

void pic_enable(uint8_t irq) {
  uint8_t mask;
  if (irq < 8) {
    mask = inb(MASTER_DATA);
    mask = mask & ~(1 << irq);
    outb(mask, MASTER_DATA);
  } else {
    irq -= 8;
    mask = inb(SLAVE_DATA);
    mask = mask & ~(1 << irq);
    outb(mask, SLAVE_DATA);
    pic_enable(2);
  }
}

void pic_disable(uint8_t irq) {
  uint8_t mask;
  if (irq < 8) {
    mask = inb(MASTER_DATA);
    mask = mask | (1 << irq);
    outb(mask, MASTER_DATA);
  } else {
    irq -= 8;
    mask = inb(SLAVE_DATA);
    mask = mask | (1 << irq);
    outb(mask, SLAVE_DATA);
  }
}

void pic_acknowledge(uint8_t irq) {
  if (irq >= 8) {
    outb(PIC_ACK_SPECIFIC + (irq - 8), SLAVE_CMD);
    outb(PIC_ACK_SPECIFIC + (2), MASTER_CMD);
  } else {
    outb(PIC_ACK_SPECIFIC + irq, MASTER_CMD);
  }
}
