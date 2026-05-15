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
#include "timer.h"
#include "sleep.h"

uint16_t video_xbytes = 1024 * 3, video_xres = 1024, video_yres = 768;
uint8_t* video_buffer = (uint8_t*) 0xA0000;

struct ap_boot_info* boot_info = (struct ap_boot_info*) AP_BOOT_INFO_ADDR;

unsigned char tmp_stack[4096] = {0};

void ap_kernel_main();

void kernel_main() {

    cli();

    VbeModeInfoBlock *video_mode = (VbeModeInfoBlock*)boot_info->vesa_info;

    video_xres = video_mode->XResolution;
    video_yres = video_mode->YResolution;
    video_xbytes = video_mode->BytesPerScanLine;
    video_buffer = (uint8_t*)(uintptr_t)video_mode->PhysBasePtr;
    
    pt_setup();
    kmalloc_init((char*) KMALLOC_START, KMALLOC_LENGTH);

    console_t console = {
        .width = video_xres / 8,
        .height = video_yres / 8,
        .x = 0,
        .y = 0,
        .buffer = 0
    };
    console_setref(&console);
    printf("Console initialized.\n");
    printf("VESA buffer: %p\n", ((VbeModeInfoBlock*)boot_info->vesa_info)->PhysBasePtr);
    
    boot_info->kernel_entry = (uint32_t) &ap_kernel_main;
    boot_info->stack_ptr = &tmp_stack[4095];

    struct cpu_features cpu_features = __get_cpu_features();
    
    uint64_t rsdp = find_rsdp();
    printf("Found RSDP at %p\n", rsdp);
    rsdp_setup((struct XSDP_t*) rsdp);
    printf("Found RSDT at %p\n", RSDT);
    printf("Found MADT at %p\n", MADT);

    enumerate_madt_cores();
    
    struct gdt_entry* gdt = kmalloc(sizeof(struct gdt_entry) * 5);
    gdt_setup(gdt, 5);

    interrupts_setup();
    struct idt* idt = kmalloc(sizeof(struct idt));
    idt_setup(idt);
    apic_setup();
    sti();

    timer_setup();
    
    //smp_setup();

    for(;;){
        
    }
}

void ap_kernel_main() {

    boot_info->ap_startup_done = 1;

    for(;;){
        
    }
}