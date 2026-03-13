#include <stdint.h>
#include "memlayout.h"
#include "kmalloc.h"
#include "printf.h"
#include "string.h"
#include "console.h"
#include "graphics.h"
#include "globals.h"
#include "vesa.h"
#include "cpu.h"
#include "assert.h"
#include "gdt.h"
#include "idt.h"
#include "interrupt.h"
#include "apic.h"
#include "page.h"
#include "smp.h"
#include "sdt.h"
#include "vesa.h"
#include "io.h"

uint16_t video_xbytes = 1024 * 3, video_xres = 1024, video_yres = 768;
uint8_t* video_buffer = (uint8_t*) 0xA0000;

struct ap_boot_info* boot_info = (struct ap_boot_info*) AP_BOOT_INFO_ADDR;

unsigned char tmp_stack[4096] = {0};

void ap_kernel_main();

void kernel_main() {

    console_t console = {
        .width = video_xres / 8,
        .height = video_yres / 8,
        .x = 0,
        .y = 0,
        .buffer = 0
    };
    console_setref(&console);
    printf("Console initialized.\n");

    kmalloc_init((char*) KMALLOC_START, KMALLOC_LENGTH);

    boot_info->kernel_entry = (uint32_t) &ap_kernel_main;
    boot_info->stack_ptr = &tmp_stack[4095];

    struct cpu_features cpu_features = __get_cpu_features();

    uint64_t rsdp = find_rsdp();
    rsdp_setup((struct XSDP_t*) rsdp);

    pt_setup();

    idt_setup();
    apic_setup();
    sti();

    smp_setup();

    for(;;){
        
    }
}

void ap_kernel_main() {
    for(;;){
        
    }
}