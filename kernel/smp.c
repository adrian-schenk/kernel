#include "smp.h"
#include "apic.h"
#include "boot_info.h"

void smp_setup() {

    int target_apic_id = 1; // TODO: Get this from ACPI MADT

    boot_info->cpu_id = target_apic_id;

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
}