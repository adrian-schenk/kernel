#pragma once
#include <stdint.h>
#include "boot_info.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4
#define PAGE_PWT 0x8
#define PAGE_PCD 0x10
#define PAGE_ACCESSED 0x20
#define PAGE_DIRTY 0x40
#define PAGE_HUGE 0x80
#define PAGE_GLOBAL 0x100
#define PAGE_NO_EXECUTE 0x8000000000000000

#define PAGE_SIZE 4096
#define HUGE_PAGE_SIZE 0x200000

#define IDX_PML4(v) (((v) >> 39) & 0x1FF)
#define IDX_PDPT(v) (((v) >> 30) & 0x1FF)
#define IDX_PD(v)   (((v) >> 21) & 0x1FF)
#define IDX_PT(v)   (((v) >> 12) & 0x1FF)

struct page_table_entry {
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

struct page_table {
    struct page_table_entry entries[512];
} __attribute__((packed));

extern struct page_table _l1 __attribute__((aligned(4096)));
extern struct page_table _l2 __attribute__((aligned(4096)));
extern struct page_table _l3 __attribute__((aligned(4096)));
extern struct page_table _l4 __attribute__((aligned(4096)));

extern struct page_table kernel_pml4 __attribute__((aligned(4096)));

int pt_map_page(struct page_table *pml4, uint64_t virt, uint64_t phys, uint64_t flags);
int pt_map_page_huge(struct page_table *pml4, uint64_t virt, uint64_t phys, uint64_t flags);

static struct page_table *get_next_table(struct page_table *current, uint16_t index);

void pt_clear_page(void* p);

void pt_setup();