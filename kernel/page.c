#include "page.h"

struct page_table _l1 __attribute__((aligned(4096))) = {0};
struct page_table _l2 __attribute__((aligned(4096))) = {0};
struct page_table _l3 __attribute__((aligned(4096))) = {0};
struct page_table _l4 __attribute__((aligned(4096))) = {0};

int pt_map_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint16_t i_pt   = (virt >> 12) & 0x1FF;
    uint16_t i_pd   = (virt >> 21) & 0x1FF;
    uint16_t i_pdpt = (virt >> 30) & 0x1FF;
    uint16_t i_pml4 = (virt >> 39) & 0x1FF;

    _l1.entries[i_pml4].present = flags & PAGE_PRESENT ? 1 : 0;
    _l1.entries[i_pml4].writable = flags & PAGE_WRITABLE ? 1 : 0;
    _l1.entries[i_pml4].user = flags & PAGE_USER ? 1 : 0;
    _l1.entries[i_pml4].pwt = flags & PAGE_PWT ? 1 : 0;
    _l1.entries[i_pml4].pcd = flags & PAGE_PCD ? 1 : 0;
    _l1.entries[i_pml4].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;
    _l1.entries[i_pml4].address = ((uint64_t)&_l2) >> 12;

    _l2.entries[i_pdpt].present = flags & PAGE_PRESENT ? 1 : 0;
    _l2.entries[i_pdpt].writable = flags & PAGE_WRITABLE ? 1 : 0;
    _l2.entries[i_pdpt].user = flags & PAGE_USER ? 1 : 0;
    _l2.entries[i_pdpt].pwt = flags & PAGE_PWT ? 1 : 0;
    _l2.entries[i_pdpt].pcd = flags & PAGE_PCD ? 1 : 0;
    _l2.entries[i_pdpt].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;
    _l2.entries[i_pdpt].address = ((uint64_t)&_l3) >> 12;

    _l3.entries[i_pd].present = flags & PAGE_PRESENT ? 1 : 0;
    _l3.entries[i_pd].writable = flags & PAGE_WRITABLE ? 1 : 0;
    _l3.entries[i_pd].user = flags & PAGE_USER ? 1 : 0;
    _l3.entries[i_pd].pwt = flags & PAGE_PWT ? 1 : 0;
    _l3.entries[i_pd].pcd = flags & PAGE_PCD ? 1 : 0;
    _l3.entries[i_pd].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;
    _l3.entries[i_pd].address = ((uint64_t)&_l4) >> 12;

    _l4.entries[i_pt].present = flags & PAGE_PRESENT ? 1 : 0;
    _l4.entries[i_pt].writable = flags & PAGE_WRITABLE ? 1 : 0;
    _l4.entries[i_pt].user = flags & PAGE_USER ? 1 : 0;
    _l4.entries[i_pt].pwt = flags & PAGE_PWT ? 1 : 0;
    _l4.entries[i_pt].pcd = flags & PAGE_PCD ? 1 : 0;
    _l4.entries[i_pt].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;
    _l4.entries[i_pt].address = phys >> 12;
    
    return 0;
}

int pt_map_page_huge(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint16_t i_pd   = (virt >> 21) & 0x1FF;
    uint16_t i_pdpt = (virt >> 30) & 0x1FF;
    uint16_t i_pml4 = (virt >> 39) & 0x1FF;

    _l1.entries[i_pml4].present = flags & PAGE_PRESENT ? 1 : 0;
    _l1.entries[i_pml4].writable = flags & PAGE_WRITABLE ? 1 : 0;
    _l1.entries[i_pml4].user = flags & PAGE_USER ? 1 : 0;
    _l1.entries[i_pml4].pwt = flags & PAGE_PWT ? 1 : 0;
    _l1.entries[i_pml4].pcd = flags & PAGE_PCD ? 1 : 0;
    _l1.entries[i_pml4].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;
    _l1.entries[i_pml4].address = ((uint64_t)&_l2) >> 12;

    _l2.entries[i_pdpt].present = flags & PAGE_PRESENT ? 1 : 0;
    _l2.entries[i_pdpt].writable = flags & PAGE_WRITABLE ? 1 : 0;
    _l2.entries[i_pdpt].user = flags & PAGE_USER ? 1 : 0;
    _l2.entries[i_pdpt].pwt = flags & PAGE_PWT ? 1 : 0;
    _l2.entries[i_pdpt].pcd = flags & PAGE_PCD ? 1 : 0;
    _l2.entries[i_pdpt].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;
    _l2.entries[i_pdpt].address = ((uint64_t)&_l3) >> 12;

    _l3.entries[i_pd].present = flags & PAGE_PRESENT ? 1 : 0;
    _l3.entries[i_pd].writable = flags & PAGE_WRITABLE ? 1 : 0;
    _l3.entries[i_pd].user = flags & PAGE_USER ? 1 : 0;
    _l3.entries[i_pd].pwt = flags & PAGE_PWT ? 1 : 0;
    _l3.entries[i_pd].pcd = flags & PAGE_PCD ? 1 : 0;
    _l3.entries[i_pd].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;
    _l3.entries[i_pd].huge_page = 1;
    _l3.entries[i_pd].address = phys >> 12;

    return 0;
}

void pt_setup() {

    for (int i = 0; i < 0x4000000; i+=0x200000) {
        pt_map_page_huge(i,i,PAGE_PRESENT | PAGE_WRITABLE);
    }

    asm volatile (
        "mov %0, %%cr3"
        :        
        : "r" (&_l1)
        : "memory"
    );
}