#include "smp.h"
#include "apic.h"
#include "boot_info.h"
#include "printf.h"

void smp_setup() {

    unsigned char stacks[apic_ids_count * 4096];
    for (int i = 1; i < apic_ids_count; i++) {
        printf("Starting CPU %d of %d\n", i, apic_ids_count - 1);
        boot_info->ap_startup_done = 0;
        int target_apic_id = apic_ids[i];

        boot_info->cpu_id = i;
        boot_info->stack_ptr = &stacks[4096 * i];
        printf("stack-ptr for core: %p\n", boot_info->stack_ptr);
        apic_write(0x280,0); // clear apic errors
        apic_write(0x310, (apic_read(0x310) & 0x00ffffff) | (target_apic_id << 24)); // Select AP
        apic_write(0x300, (apic_read(0x300) & 0xfff00000) | 0x00C500); // Trigger INIT IPI
        while (apic_read(0x300) & 0x1000); // Wait for delivery
        apic_write(0x310, (apic_read(0x310) & 0x00ffffff) | (target_apic_id << 24)); // Select AP
        apic_write(0x300, (apic_read(0x300) & 0xfff00000) | 0x008500); // Deassert
        while (apic_read(0x300) & 0x1000); // Wait for delivery
    
        apic_write(0x280, 0); // Clear APIC errors
        apic_write(0x310, (apic_read(0x310) & 0x00ffffff) | (target_apic_id << 24)); // Select AP
        apic_write(0x300, (apic_read(0x300) & 0xfff0f800) | 0x000610); // Trigger STARTUP IPI for 1000:0000

        apic_write(EOI_REGISTER, 0);

        while (boot_info->ap_startup_done == 0);
    }
}