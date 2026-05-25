#pragma once
#include <interrupt.h>
#include <io.h>

typedef struct {
    char shift;
    char ctrl;
    char altgr;
} keyboardstate_t;

void keyboard_init();

void keyboard_interrupt_handler(uint64_t interrupt_number, uint64_t error_code);

char translate(char c, int shift, int ctrl, int altgr);