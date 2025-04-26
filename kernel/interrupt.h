#pragma once
#include <types.h>  

typedef void (*int_handler)(int i, int code);

void interrupt_init();
char interrupt_register(int i, int_handler);
void interrupt_handler(int i, int code);

void interrupt_block();
void interrupt_unblock();
