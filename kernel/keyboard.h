#pragma once
#include "interrupt.h"
#include "ioports.h"

void keyboard_init();
void keyboard_handler(int i, int code);
