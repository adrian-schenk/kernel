#include "apic.h"
#include "io.h"
#include "cpu.h"
#include "page.h"
#include "mmio.h"

#define APIC_VIRT 0xFFEE0000
#define IOAPIC_VIRT 0xFFEF0000

uint64_t apic_base;
uint64_t ioapic_base;

void apic_setup() {
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);   

    if (__check_apic()) {
        uint64_t apic_info = __read_msr(0x1B);
        apic_base = apic_info;

        pt_map_page(APIC_VIRT, apic_base, PAGE_PRESENT | PAGE_WRITABLE | PAGE_PWT | PAGE_PCD);

        uint32_t svr = mmio_readl(APIC_VIRT + 0xF0);
        svr |= 0x100; // Set the APIC Software Enable/Disable bit (bit 8) to enable the APIC.
        svr |= 0xFF; // Set the APIC Version to 0xFF to indicate that the APIC supports xAPIC mode.
        mmio_writel(APIC_VIRT + 0xF0, svr);

        mmio_writel(APIC_VIRT + 0xD0, 0x10000000); // set apic id
        mmio_writel(APIC_VIRT + 0xE0, 0xF0000000); // flat model
    } else {
        // HLT
        for(;;);
    }

    pt_map_page(IOAPIC_VIRT, ioapic_base, PAGE_PRESENT | PAGE_WRITABLE | PAGE_PWT | PAGE_PCD);
}

void apic_write(uint16_t reg, uint32_t value) {
    mmio_writel(APIC_VIRT + reg, value);
}

uint32_t apic_read(uint16_t reg) {
    return mmio_readl(APIC_VIRT + reg);
}

uint32_t ioapic_read(uint32_t reg) {
    uint32_t volatile *ioapic = (uint32_t volatile*) IOAPIC_VIRT;
    ioapic[0] = (reg & 0xff);
    return ioapic[4];
}

void ioapic_write(uint32_t reg, uint32_t value) {
    uint32_t volatile*ioapic = (uint32_t volatile*) IOAPIC_VIRT;
    ioapic[0] = (reg & 0xff);
    ioapic[4] = value;
}