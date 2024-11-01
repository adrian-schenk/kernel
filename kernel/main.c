#include <stdint.h>
#include "memlayout.h"
#include "kmalloc.h"
#include "printf.h"
#include "string.h"
#include "console.h"
#include "graphics.h"
#include "globals.h"
#include "vesa.h"

void kernel_main() {

    kmalloc_init((char*) KMALLOC_START, KMALLOC_LENGTH);

    console_t* console = console_init();
    console_setref(console);

    vbe_info_structure* vbeinfo = 0x1005c;
    VbeModeInfoBlock *info = 0x10258;

    video_buffer = info->PhysBasePtr;

    printf("%s\n", &vbeinfo->signature);
    printf("%x\n", vbeinfo->version);

    printf("%x %u\n", info->PhysBasePtr, info->BitsPerPixel);
    printf("%u %u %u\n", info->RedMaskSize, info->GreenMaskSize, info->BlueMaskSize);
    printf("%u %u %u\n", info->RedFieldPosition, info->GreenFieldPosition, info->BlueFieldPosition);
    printf("%u \n", info->BytesPerScanLine);
    
    for(int i = 0; i < video_yres; i++){
        graphics_draw_pixel(i,i, 0xffffff00);
    }
    for(;;){
        
    }
}