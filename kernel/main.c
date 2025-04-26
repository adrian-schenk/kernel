#include <stdint.h>
#include "interrupt.h"
#include "keyboard.h"
#include "memlayout.h"
#include "kmalloc.h"
#include "printf.h"
#include "string.h"
#include "console.h"
#include "graphics.h"
#include "globals.h"
#include "vesa.h"
#include "pic.h"

uint16_t video_xbytes = 3072;
uint16_t video_xres = 1024;
uint16_t video_yres = 768;
uint8_t* video_buffer = 0xFD000000;

void kernel_main() {

    kmalloc_init((char*) KMALLOC_START, KMALLOC_LENGTH);

    console_t* console = console_init();
    console_setref(console);

    pic_init(32,40);
    
    interrupt_init();
    keyboard_init();

    for(int i = 0; i < video_yres; i++){
        graphics_draw_pixel(i,i, 0xffffffff);
    }
    for(;;){
        
    }
}
