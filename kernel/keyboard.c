#include "keyboard.h"
#include "printf.h"
#include <ascii.h>

#define KEYBOARD_PORT 0x60

#define KEYMAP_SHIFT 1
#define KEYMAP_ALT 2
#define KEYMAP_CTRL 3
#define KEYMAP_CAPSLOCK 4
#define KEYMAP_NUMLOCK 5
#define KEYMAP_ALPHA 6
#define KEYMAP_NUMPAD 8

/* sent before certain keys such as up, down, left, or right. */
#define KEYCODE_EXTRA (uint8_t)0xE0
#define KEYCODE_UP (uint8_t)0x48
#define KEYCODE_DOWN (uint8_t)0x42
#define KEYCODE_LEFT (uint8_t)0x4B
#define KEYCODE_RIGHT (uint8_t)0x4D

#define BUFFER_SIZE 256

struct keymap {
  char normal;
  char shifted;
  char ctrled;
  char special;
};

static struct keymap keymap[] = {
#include "keymap.h";
};

void keyboard_init() { interrupt_register(33, keyboard_handler); }

void keyboard_handler(int i, int code) {
  int keycode = inb(0x60);
  // printf("%x", keycode);
  if (!(keycode & 0x80)) {
    printf("%c", keymap[keycode].normal);
  }
}
