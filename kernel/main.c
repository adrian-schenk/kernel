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
#include "task.h"
#include "scheduler.h"
#include "pci.h"
#include <keyboard.h>

uint16_t video_xbytes = 1024 * 3, video_xres = 1024, video_yres = 768;
uint8_t* video_buffer = (uint8_t*) 0xA0000;

struct cpu_local *cpu_locals;
int cpu_count = 0;

volatile int timer_phase = 0;

struct ap_boot_info* boot_info = (struct ap_boot_info*) AP_BOOT_INFO_ADDR;

void ap_kernel_main();

void thread_idle() {
    for(;;) {
        
    }
}

void kernel_main() {

    cli();

    VbeModeInfoBlock *video_mode = (VbeModeInfoBlock*)boot_info->vesa_info;

    video_xres = video_mode->XResolution;
    video_yres = video_mode->YResolution;
    video_xbytes = video_mode->BytesPerScanLine;
    video_buffer = (uint8_t*)(uintptr_t)video_mode->PhysBasePtr;
    
    pt_setup();
    kmalloc_init((char*) KMALLOC_START, KMALLOC_LENGTH);
    
    struct gdt_entry* gdt = kmalloc(sizeof(struct gdt_entry) * 5);
    gdt_setup(gdt, 5);
    
    interrupts_setup();
    struct idt* idt = kmalloc(sizeof(struct idt));
    idt_setup(idt);
    apic_setup();
    ioapic_setup();

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

    uint64_t rsdp = find_rsdp();
    printf("Found RSDP at %p\n", rsdp);
    rsdp_setup((struct XSDP_t*) rsdp);
    printf("Found RSDT at %p\n", RSDT);
    printf("Found MADT at %p\n", MADT);

    enumerate_madt_cores();

    if (apic_ids_count > 0)
        cpu_locals = kmalloc(apic_ids_count * sizeof(struct cpu_local));

    struct cpu_local *cpu_local = &cpu_locals[cpu_count++];
    
    cpu_local->cpu_id = 0;
    cpu_local->interrupt_handlers = &interrupt_handlers;
    cpu_local->apic_id = apic_read(LAPIC_ID_REGISTER) >> 24;

    __write_msr(MSR_GS_BASE, (uint64_t)cpu_local); // write locals before interrupts are enabled

    keyboard_init();
    
    pci_init();
    
    sti();
    
    timer_setup();
    
    smp_setup();

    timer_phase = 1;
    sleep(timer_calib_ms);
    timer_phase = 2;

    cli();
    cpu_local->scheduler = scheduler_init();
    scheduler_add_task(cpu_local->scheduler, task_create((uint64_t) thread_idle));
    sti();

    for(;;){
        
    }
}

void ap_kernel_main() {

    cli();
    
    struct cpu_local *cpu_local = &cpu_locals[cpu_count++];
    
    cpu_local->cpu_id = boot_info->cpu_id;
    cpu_local->kernel_stack = boot_info->stack_ptr;

    cpu_enable_sse();

    struct gdt_entry* gdt = kmalloc(GDT_ENTRY_SIZE * (GDT_ENTRY * 3 + GDT_TSS_ENTRY * 1));
    gdt_setup(gdt, 5);

    struct idt* idt = kmalloc(sizeof(struct idt));
    idt_setup(idt);

    // switch interrupt_handlers to ap setup handlers temporarily
    cpu_local->interrupt_handlers = &interrupt_handlers_ap;    
    cpu_local->apic_id = apic_read(LAPIC_ID_REGISTER) >> 24;

    __write_msr(MSR_GS_BASE, (uint64_t)cpu_local);

    apic_enable(); // software-enable this AP's local APIC before any APIC writes
    
    sti();
    
    timer_setup_ap(cpu_local);

    cli();
    cpu_local->scheduler = scheduler_init();
    scheduler_add_task(cpu_local->scheduler, task_create((uint64_t) thread_idle));
    sti();

    for(;;){
        
    }
}