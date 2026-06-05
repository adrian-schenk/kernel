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
#include "ahci.h"
#include "buddy_alloc.h"
#include "ramfs.h"
#include <blkdev.h>
#include <ext4/ext4.h>
#include <keyboard.h>
#include "syscall.h"

uint16_t video_xbytes = 1024 * 3, video_xres = 1024, video_yres = 768;
uint8_t *video_buffer = (uint8_t *)0xA0000;

struct cpu_local *cpu_locals;
int cpu_count = 0;

volatile int timer_phase = 0;

struct ap_boot_info *boot_info = (struct ap_boot_info *)AP_BOOT_INFO_ADDR;

void ap_kernel_main();

void thread_idle()
{
    kprintf("thread idle starting\n");
    for (;;)
    {
        asm __volatile__("hlt");
    }
}

void thread_userspace()  {
    int i = syscall_printf("Hello from userspace!\n");
    for (;;) {
        
    }
}


void kernel_main()
{

    cli();

    VbeModeInfoBlock *video_mode = (VbeModeInfoBlock *)boot_info->vesa_info;

    video_xres = video_mode->XResolution;
    video_yres = video_mode->YResolution;
    video_xbytes = video_mode->BytesPerScanLine;
    video_buffer = (uint8_t *)(uintptr_t)video_mode->PhysBasePtr;

    pt_setup();
    kmalloc_init((char *)KMALLOC_START, KMALLOC_LENGTH);
    buddy_init();

    uint32_t *memory_map = (uint32_t *)0x7000;
    int memory_map_len = (*((uint32_t *)memory_map++));

    struct gdt_entry *gdt = kmalloc_early(GDT_ENTRY_SIZE * (GDT_ENTRY * 5 + GDT_TSS_ENTRY * 1));
    gdt_setup(gdt, (GDT_ENTRY * 5 + GDT_TSS_ENTRY * 1));

    interrupts_setup();
    struct idt *idt = kmalloc_early(sizeof(struct idt));
    idt_setup(idt);
    apic_setup();
    ioapic_setup();

    console_t console = {
        .width = video_xres / 8,
        .height = video_yres / 8,
        .x = 0,
        .y = 0,
        .buffer = 0};
    console_setref(&console);
    kprintf("Console initialized.\n");
    kprintf("Memory map length: %d, at 0x%x\n", memory_map_len, memory_map);
    for (int i = 0; i < memory_map_len; i++)
    {
        uint32_t base_addr_low = *memory_map++;
        uint32_t base_addr_high = *memory_map++;
        uint64_t base_addr = ((uint64_t)base_addr_high << 32) | base_addr_low;
        uint32_t length_low = *memory_map++;
        uint32_t length_high = *memory_map++;
        uint64_t length = ((uint64_t)length_high << 32) | length_low;
        uint32_t type = *memory_map++;
        const char *type_str;
        switch (type)
        {
        case 1:
            type_str = "Usable";
            break;
        case 2:
            type_str = "Reserved";
            break;
        case 3:
            type_str = "ACPI Reclaimable";
            break;
        case 4:
            type_str = "ACPI NVS";
            break;
        case 5:
            type_str = "Bad Memory";
            break;
        default:
            type_str = "Unknown";
            break;
        }
        kprintf("Memory region %d: Base Address: %p, Length: %p, Type: %s\n", i, base_addr, length, type_str);
    }
    kprintf("VESA buffer: %p\n", ((VbeModeInfoBlock *)boot_info->vesa_info)->PhysBasePtr);
    boot_info->kernel_entry = (uint32_t)&ap_kernel_main;

    uint64_t rsdp = find_rsdp();
    kprintf("Found RSDP at %p\n", rsdp);
    rsdp_setup((struct XSDP_t *)rsdp);
    kprintf("Found RSDT at %p\n", RSDT);
    kprintf("Found MADT at %p\n", MADT);

    enumerate_madt_cores();

    if (apic_ids_count > 0)
        cpu_locals = kmalloc_early(apic_ids_count * sizeof(struct cpu_local));

    struct cpu_local *cpu_local = &cpu_locals[cpu_count++];

    cpu_local->cpu_id = 0;
    cpu_local->interrupt_handlers = &interrupt_handlers;
    cpu_local->apic_id = apic_read(LAPIC_ID_REGISTER) >> 24;
    cpu_local->scheduler = (void *)0;

    __write_msr(MSR_GS_BASE, (uint64_t)cpu_local); // write locals before interrupts are enabled

    keyboard_init();

    pci_init();
    ahci_init();

    sti();

    fs_handle_t *ext4 = ext4_handle_create(ahci_blkdev_create(0));
    ext4->mount(ext4);

    timer_setup();

    smp_setup();

    timer_phase = 1;
    sleep(timer_calib_ms);
    timer_phase = 2;

    cli();
    cpu_local->scheduler = scheduler_init();
    scheduler_add_task(cpu_local->scheduler, task_create((uint64_t)thread_userspace));
    scheduler_add_task(cpu_local->scheduler, task_create_priv((uint64_t)thread_idle, 0x10, 0x8));
    sti();

    for (;;)
    {
    }
}

void ap_kernel_main()
{

    cli();

    struct cpu_local *cpu_local = &cpu_locals[cpu_count++];

    cpu_local->cpu_id = boot_info->cpu_id;
    cpu_local->kernel_stack = boot_info->stack_ptr;

    cpu_enable_sse();

    struct gdt_entry *gdt = kmalloc_early(GDT_ENTRY_SIZE * (GDT_ENTRY * 5 + GDT_TSS_ENTRY * 1));
    gdt_setup(gdt, (GDT_ENTRY * 5 + GDT_TSS_ENTRY * 1));

    struct idt *idt = kmalloc_early(sizeof(struct idt));
    idt_setup(idt);

    asm volatile (
        "mov %0, %%cr3"
        :        
        : "r" (&kernel_pml4)
        : "memory"
    );

    // switch interrupt_handlers to ap setup handlers temporarily
    cpu_local->interrupt_handlers = &interrupt_handlers_ap;
    cpu_local->apic_id = apic_read(LAPIC_ID_REGISTER) >> 24;

    __write_msr(MSR_GS_BASE, (uint64_t)cpu_local);

    apic_enable(); // software-enable this AP's local APIC before any APIC writes

    sti();

    timer_setup_ap(cpu_local);

    cli();
    cpu_local->scheduler = scheduler_init();
    scheduler_add_task(cpu_local->scheduler, task_create_priv((uint64_t)thread_idle, 0x10, 0x8));
    scheduler_add_task(cpu_local->scheduler, task_create((uint64_t)thread_userspace));
    sti();

    for (;;)
    {
    }
}