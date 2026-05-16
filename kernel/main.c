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

uint16_t video_xbytes = 1024 * 3, video_xres = 1024, video_yres = 768;
uint8_t* video_buffer = (uint8_t*) 0xA0000;

struct cpu_local *cpu_locals;
int cpu_count = 0;

struct ap_boot_info* boot_info = (struct ap_boot_info*) AP_BOOT_INFO_ADDR;

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

    struct cpu_features cpu_features = __get_cpu_features();
    
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
    
    struct gdt_entry* gdt = kmalloc(sizeof(struct gdt_entry) * 5);
    gdt_setup(gdt, 5);
    
    interrupts_setup();
    struct idt* idt = kmalloc(sizeof(struct idt));
    idt_setup(idt);
    apic_setup();
    ioapic_setup();
    
    cpu_local->apic_id = apic_read(LAPIC_ID_REGISTER);

    __write_msr(MSR_GS_BASE, (uint64_t)cpu_local); // write locals before interrupts are enabled

    sti();
    
    timer_setup();

    smp_setup();

    for(;;){
        
    }
}

void ap_kernel_main() {

    cli();

    struct cpu_local *cpu_local = &cpu_locals[cpu_count++];
    
    cpu_local->cpu_id = boot_info->cpu_id;
    cpu_local->kernel_stack = boot_info->stack_ptr;

    // Enable x87/SSE in control registers before executing C code that may use float ops.
    __asm__ volatile (
        "mov %%cr0, %%rax\n"
        "btr $2, %%rax\n"      // CR0.EM = 0 (do not emulate x87)
        "btr $3, %%rax\n"      // CR0.TS = 0 (task-switched off)
        "bts $1, %%rax\n"      // CR0.MP = 1 (monitor coprocessor)
        "mov %%rax, %%cr0\n"
        
        "mov %%cr4, %%rax\n"
        "bts $9, %%rax\n"      // CR4.OSFXSR = 1 (enable SSE instructions)
        "bts $10, %%rax\n"     // CR4.OSXMMEXCPT = 1 (enable SSE exceptions)
        "mov %%rax, %%cr4\n"
        
        "fninit\n"
        : : : "rax", "memory"
    );

    struct idt* idt = kmalloc(sizeof(struct idt));
    idt_setup(idt);

    struct gdt_entry* gdt = kmalloc(GDT_ENTRY_SIZE * (GDT_ENTRY * 3 + GDT_TSS_ENTRY * 1));
    gdt_setup(gdt, 5);

    __write_msr(MSR_GS_BASE, (uint64_t)cpu_local);

    boot_info->ap_startup_done = 1; // boot up other cores

    sti();

    for(;;){
        
    }
}