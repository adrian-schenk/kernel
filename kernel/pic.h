#ifndef PIC_H
#define PIC_H
#include "ioports.h"
#include <stdint.h>

void pic_init(int, int);
void pic_acknowledge(uint8_t irq);
void pic_enable(uint8_t);
void pic_disable(uint8_t);

#endif


