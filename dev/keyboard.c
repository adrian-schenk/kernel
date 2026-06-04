#include "keyboard.h"
#include <printf.h>

keyboardstate_t keyboard_state = {};

typedef struct {
    char normal;
    char shift;
    char ctrl;
    char altgr;
} keymap_t;

static const keymap_t keymap[256] = {
    ['1'] = { '1', '!', 0, 0 },
    ['2'] = { '2', '@', 0, 0 },
    ['3'] = { '3', '#', 0, 0 },
    ['4'] = { '4', '$', 0, 0 },
    ['5'] = { '5', '%', 0, 0 },
    ['6'] = { '6', '^', 0x1E, 0 }, // Ctrl-^
    ['7'] = { '7', '&', 0, '{' },
    ['8'] = { '8', '*', 0x08, '[' },
    ['9'] = { '9', '(', 0, ']' },
    ['0'] = { '0', ')', 0, '}' },

    ['-'] = { '-', '_', 0x1F, '\\' },
    ['='] = { '=', '+', 0, '~' },

    ['`'] = { '`', '~', 0, 0 },

    ['['] = { '[', '{', 0x1B, 0 },
    [']'] = { ']', '}', 0x1D, 0 },
    ['\\'] = { '\\', '|', 0x1C, 0 },

    [';'] = { ';', ':', 0, 0 },
    ['\''] = { '\'', '"', 0, 0 },

    [','] = { ',', '<', 0, 0 },
    ['.'] = { '.', '>', 0, 0 },
    ['/'] = { '/', '?', 0x7F, 0 },

    ['a'] = { 'a', 'A', 0x01, 0 },
    ['b'] = { 'b', 'B', 0x02, 0 },
    ['c'] = { 'c', 'C', 0x03, 0 },
    ['d'] = { 'd', 'D', 0x04, 0 },
    ['e'] = { 'e', 'E', 0x05, 0 },
    ['f'] = { 'f', 'F', 0x06, 0 },
    ['g'] = { 'g', 'G', 0x07, 0 },
    ['h'] = { 'h', 'H', 0x08, 0 },
    ['i'] = { 'i', 'I', 0x09, 0 },
    ['j'] = { 'j', 'J', 0x0A, 0 },
    ['k'] = { 'k', 'K', 0x0B, 0 },
    ['l'] = { 'l', 'L', 0x0C, 0 },
    ['m'] = { 'm', 'M', 0x0D, 0 },
    ['n'] = { 'n', 'N', 0x0E, 0 },
    ['o'] = { 'o', 'O', 0x0F, 0 },
    ['p'] = { 'p', 'P', 0x10, 0 },
    ['q'] = { 'q', 'Q', 0x11, 0 },
    ['r'] = { 'r', 'R', 0x12, 0 },
    ['s'] = { 's', 'S', 0x13, 0 },
    ['t'] = { 't', 'T', 0x14, 0 },
    ['u'] = { 'u', 'U', 0x15, 0 },
    ['v'] = { 'v', 'V', 0x16, 0 },
    ['w'] = { 'w', 'W', 0x17, 0 },
    ['x'] = { 'x', 'X', 0x18, 0 },
    ['y'] = { 'y', 'Y', 0x19, 0 },
    ['z'] = { 'z', 'Z', 0x1A, 0 },

    [' '] = { ' ', ' ', 0, 0 },
};

typedef enum {
    KEY_NONE = 0,

    KEY_ESC,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_ENTER,

    KEY_CTRL,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_ALT,
    KEY_CAPSLOCK,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_NUMLOCK,
    KEY_SCROLLLOCK,

    KEY_HOME,
    KEY_UP,
    KEY_PAGEUP,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_END,
    KEY_DOWN,
    KEY_PAGEDOWN,
    KEY_INSERT,
    KEY_DELETE,

    KEY_GUI_LEFT,
    KEY_GUI_RIGHT,
    KEY_APPS,

    KEY_PRINTSCREEN,
    KEY_PAUSE,

    KEY_POWER,
    KEY_SLEEP,
    KEY_WAKE,

    KEY_MEDIA_PREV,
    KEY_MEDIA_NEXT,
    KEY_MEDIA_MUTE,
    KEY_MEDIA_CALC,
    KEY_MEDIA_PLAY,
    KEY_MEDIA_STOP,
    KEY_MEDIA_VOL_DOWN,
    KEY_MEDIA_VOL_UP,
    KEY_MEDIA_WWW_HOME,
    KEY_MEDIA_WWW_SEARCH,
    KEY_MEDIA_WWW_FAVORITES,
    KEY_MEDIA_WWW_REFRESH,
    KEY_MEDIA_WWW_STOP,
    KEY_MEDIA_WWW_FORWARD,
    KEY_MEDIA_WWW_BACK,
    KEY_MEDIA_MY_COMPUTER,
    KEY_MEDIA_EMAIL,
    KEY_MEDIA_SELECT,
} keycode_t;

static const int scancode_set1[256] = {
    [0x01] = KEY_ESC,

    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',

    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = KEY_BACKSPACE,
    [0x0F] = KEY_TAB,

    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',

    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = KEY_ENTER,
    [0x1D] = KEY_CTRL,

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',

    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',

    [0x2A] = KEY_LSHIFT,
    [0x2B] = '\\',

    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',

    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',

    [0x36] = KEY_RSHIFT,
    [0x37] = '*',

    [0x38] = KEY_ALT,
    [0x39] = ' ',

    [0x3A] = KEY_CAPSLOCK,

    [0x3B] = KEY_F1,
    [0x3C] = KEY_F2,
    [0x3D] = KEY_F3,
    [0x3E] = KEY_F4,
    [0x3F] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,

    [0x45] = KEY_NUMLOCK,
    [0x46] = KEY_SCROLLLOCK,

    [0x47] = '7',
    [0x48] = '8',
    [0x49] = '9',
    [0x4A] = '-',
    [0x4B] = '4',
    [0x4C] = '5',
    [0x4D] = '6',
    [0x4E] = '+',
    [0x4F] = '1',
    [0x50] = '2',
    [0x51] = '3',
    [0x52] = '0',
    [0x53] = '.',

    [0x57] = KEY_F11,
    [0x58] = KEY_F12,
};

static const int scancode_set1_e0[256] = {
    [0x10] = KEY_MEDIA_PREV,
    [0x19] = KEY_MEDIA_NEXT,

    [0x1C] = KEY_ENTER,
    [0x1D] = KEY_CTRL,

    [0x20] = KEY_MEDIA_MUTE,
    [0x21] = KEY_MEDIA_CALC,
    [0x22] = KEY_MEDIA_PLAY,
    [0x24] = KEY_MEDIA_STOP,

    [0x2E] = KEY_MEDIA_VOL_DOWN,
    [0x30] = KEY_MEDIA_VOL_UP,
    [0x32] = KEY_MEDIA_WWW_HOME,

    [0x35] = '/',
    [0x38] = KEY_ALT,

    [0x47] = KEY_HOME,
    [0x48] = KEY_UP,
    [0x49] = KEY_PAGEUP,
    [0x4B] = KEY_LEFT,
    [0x4D] = KEY_RIGHT,
    [0x4F] = KEY_END,
    [0x50] = KEY_DOWN,
    [0x51] = KEY_PAGEDOWN,
    [0x52] = KEY_INSERT,
    [0x53] = KEY_DELETE,

    [0x5B] = KEY_GUI_LEFT,
    [0x5C] = KEY_GUI_RIGHT,
    [0x5D] = KEY_APPS,

    [0x5E] = KEY_POWER,
    [0x5F] = KEY_SLEEP,
    [0x63] = KEY_WAKE,

    [0x65] = KEY_MEDIA_WWW_SEARCH,
    [0x66] = KEY_MEDIA_WWW_FAVORITES,
    [0x67] = KEY_MEDIA_WWW_REFRESH,
    [0x68] = KEY_MEDIA_WWW_STOP,
    [0x69] = KEY_MEDIA_WWW_FORWARD,
    [0x6A] = KEY_MEDIA_WWW_BACK,
    [0x6B] = KEY_MEDIA_MY_COMPUTER,
    [0x6C] = KEY_MEDIA_EMAIL,
    [0x6D] = KEY_MEDIA_SELECT,
};

void keyboard_init() {
  set_interrupt_handler(interrupt_handlers, 0x21, keyboard_interrupt_handler);
}

void keyboard_interrupt_handler(uint64_t interrupt_number, uint64_t error_code) {
    uint8_t scancode = inb(0x60);
    
    switch (scancode) {
      case 0x2A:
      case 0x36:
        keyboard_state.shift = 1;
        break;
      case 0xAA:
      case 0xB6:
        keyboard_state.shift = 0;
        break;
      case 0x1D:
        keyboard_state.ctrl = 1;
        break;
      case 0x9D:
        keyboard_state.ctrl = 0;
        break;
      case 0x38:
        keyboard_state.altgr = 1;
        break;
      case 0xB8:
        keyboard_state.altgr = 0;
        break;
    }

    if ((scancode & 0x80) == 0) {
        // Key press
        if (scancode == 0x1C) {
            kprintf("\n");
            return;
        }
        char c = translate(scancode_set1[scancode], keyboard_state.shift, keyboard_state.ctrl, keyboard_state.altgr);
        if (c) {
            kprintf("%c", c);
        }
    } else {
        // Key release
    }
}

char translate(char c, int shift, int ctrl, int altgr) {
    keymap_t k = keymap[(unsigned char)c];

    if (ctrl && k.ctrl)
        return k.ctrl;

    if (altgr && k.altgr)
        return k.altgr;

    if (shift)
        return k.shift;

    return k.normal;
}