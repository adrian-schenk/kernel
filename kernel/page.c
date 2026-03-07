#include "page.h"

struct page_table _l1 __attribute__((aligned(4096))) = {0};
struct page_table _l2 __attribute__((aligned(4096))) = {0};
struct page_table _l3 __attribute__((aligned(4096))) = {0};
struct page_table _l4 __attribute__((aligned(4096))) = {0};

int pt_map_page(uint64_t virt, uint64_t phys) {
    uint16_t i_pt   = (virt >> 12) & 0x1FF;
    uint16_t i_pd   = (virt >> 21) & 0x1FF;
    uint16_t i_pdpt = (virt >> 30) & 0x1FF;
    uint16_t i_pml4 = (virt >> 39) & 0x1FF;

    _l1.entries[i_pml4].present = 1;
    _l1.entries[i_pml4].writable = 1;
    _l1.entries[i_pml4].address = ((uint64_t)&_l2) >> 12;

    _l2.entries[i_pdpt].present = 1;
    _l2.entries[i_pdpt].writable = 1;
    _l2.entries[i_pdpt].address = ((uint64_t)&_l3) >> 12;

    _l3.entries[i_pd].present = 1;
    _l3.entries[i_pd].writable = 1;
    _l3.entries[i_pd].address = ((uint64_t)&_l4) >> 12;

    _l4.entries[i_pt].present = 1;
    _l4.entries[i_pt].writable = 1;
    _l4.entries[i_pt].address = phys >> 12;
    
    return 0;
}

int pt_map_page_huge(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint16_t i_pd   = (virt >> 21) & 0x1FF;
    uint16_t i_pdpt = (virt >> 30) & 0x1FF;
    uint16_t i_pml4 = (virt >> 39) & 0x1FF;

    _l1.entries[i_pml4].present = 1;
    _l1.entries[i_pml4].writable = 1;
    _l1.entries[i_pml4].address = ((uint64_t)&_l2) >> 12;

    _l2.entries[i_pdpt].present = 1;
    _l2.entries[i_pdpt].writable = 1;
    _l2.entries[i_pdpt].address = ((uint64_t)&_l3) >> 12;

    _l3.entries[i_pd].present = 1;
    _l3.entries[i_pd].writable = 1;
    _l3.entries[i_pd].huge_page = 1;
    _l3.entries[i_pd].address = phys >> 12;

    return 0;
}

void pt_setup() {

    for (int i = 0; i < 0x4000000; i+=0x200000) {
        pt_map_page_huge(i,i,0);
    }

    asm volatile (
        "mov %0, %%cr3"
        :        
        : "r" (&_l1)
        : "memory"
    );
}