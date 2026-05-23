#pragma once
#include <stdint.h>

extern uint16_t video_xbytes;
extern uint16_t video_xres;
extern uint16_t video_yres;
extern uint8_t *video_buffer;

static const int timer_calib_ms = 200;
extern volatile int timer_phase;