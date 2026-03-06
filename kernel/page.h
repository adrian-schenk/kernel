#pragma once
#include <stdint.h>
#include "boot_info.h"

struct page_table_entry{
    uint64_t present    : 1;   // Bit 0
    uint64_t writable   : 1;   // Bit 1
    uint64_t user       : 1;   // Bit 2
    uint64_t pwt        : 1;   // Bit 3
    uint64_t pcd        : 1;   // Bit 4
    uint64_t accessed   : 1;   // Bit 5
    uint64_t dirty      : 1;   // Bit 6
    uint64_t huge_page  : 1;   // Bit 7 (PS)
    uint64_t global     : 1;   // Bit 8
    uint64_t available  : 3;   // Bits 9-11
    uint64_t address    : 40;  // Bits 12-51 (Pointer to next level page table or physical page)
    uint64_t avl_high   : 11;  // Bits 52-62
    uint64_t no_execute : 1;   // Bit 63
} __attribute__((packed));

struct virt_address{
    uint64_t address : 48;
    uint64_t null : 16;
};

int pt_map_page(uint64_t virt, uint64_t phys, uint64_t flags);
int pt_map_page_huge(uint64_t virt, uint64_t phys, uint64_t flags);