#ifndef GDT_H
#define GDT_H
#include <stdint.h>

#define GDT_A_PRESENT       0x80
#define GDT_A_DESCRIPTOR    0x10
#define GDT_A_PRIVL_0       0x00
#define GDT_A_PRIVL_1       0x20
#define GDT_A_PRIVL_2       0x40
#define GDT_A_PRIVL_3       0x60
#define GDT_A_CODE_DATA     0x10
#define GDT_A_EXECUTABLE    0x08
#define GDT_A_DC            0x04
#define GDT_A_RW            0x02
#define GDT_A_ACCESSED      0x01
#define GDT_A_CODE         (GDT_A_PRESENT | GDT_A_PRIVL_0 | GDT_A_CODE_DATA | GDT_A_EXECUTABLE)
#define GDT_A_DATA         (GDT_A_PRESENT | GDT_A_PRIVL_0 | GDT_A_CODE_DATA)

#define GDT_F_GRANULARITY   0x80
#define GDT_F_64BIT         0x20
#define GDT_F_32BIT         0x40

struct gdt_entry {
    uint16_t limit;    // Lower 16 bits of limit
    uint16_t base_low;     // Lower 16 bits of base
    uint8_t  base_middle;  // Next 8 bits of base
    uint8_t  access;       // Access flags
    uint8_t  granularity;  // Granularity and high 4 bits of limit
    uint8_t  base_high;    // Last 8 bits of base
} __attribute__((packed));

struct gdt_entry_tss {
    uint16_t limit;    // Lower 16 bits of limit
    uint16_t base_low;     // Lower 16 bits of base
    uint8_t  base_middle;  // Next 8 bits of base
    uint8_t  access;       // Access flags
    uint8_t  granularity;  // Granularity and high 4 bits of limit
    uint8_t  base_high;    // Last 8 bits of base
    uint32_t base_upper;   // Upper 32 bits of base (only used in TSS descriptor)
    uint32_t reserved;     // Reserved, set to 0
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;        // Size of GDT in bytes
    uint64_t base;         // Base address of the first GDT entry
} __attribute__((packed));


#define SEGMENT_DESCRIPTORS 3 // descriptor count without null
#define TSS_DESCRIPTORS 1 // tss descriptor count

extern struct gdt_entry gdt[1 + SEGMENT_DESCRIPTORS + (TSS_DESCRIPTORS * 2)] __attribute__((aligned(8)));
extern struct gdt_ptr gdt_init;

void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void set_tss_entry(int num, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_setup();
void gdt_load();

#endif