#include <stdint.h>

// Define the structure of a GDT entry (8 bytes)
struct gdt_entry {
    uint16_t limit_low;    // Lower 16 bits of limit
    uint16_t base_low;     // Lower 16 bits of base
    uint8_t  base_middle;  // Next 8 bits of base
    uint8_t  access;       // Access flags
    uint8_t  granularity;  // Granularity and high 4 bits of limit
    uint8_t  base_high;    // Last 8 bits of base
} __attribute__((packed));

// Define the structure for the GDT pointer (6 bytes)
struct gdt_ptr {
    uint16_t limit;        // Size of GDT in bytes - 1
    uint32_t base;         // Base address of the first GDT entry
} __attribute__((packed));

// Define the GDT and GDT pointer
struct gdt_entry gdt[3];
struct gdt_ptr gp;

void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);

void gdt_setup();

extern void gdt_flush(uint32_t);

