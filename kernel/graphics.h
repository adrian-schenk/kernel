#pragma once

#include <stdint.h>
#include "font.h"

#include "globals.h"

typedef struct graphics
{
  uint8_t *video_buffer;
} graphics_t;

typedef struct graphics_color
{
  union {
    struct {
      uint8_t red;
      uint8_t green;
      uint8_t blue;
      uint8_t alpha;
    };
    uint32_t color;
  };
  union  {
    struct {
      uint8_t r_bg;
      uint8_t g_bg;
      uint8_t b_bg;
      uint8_t a_bg;
    };
    uint32_t bg_color;
  };
} graphics_color_t;

void graphics_draw_char(int x, int y, char c, graphics_color_t);
int graphics_draw_pixel(int x, int y, int color);
