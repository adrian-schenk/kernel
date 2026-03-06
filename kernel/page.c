#include "page.h"

int pt_map_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    virt &= 0xFFFFFFFFFF000; // Align to 4KB
    phys &= 0xFFFFFFFFFF000; // Align to 4KB

    uint16_t l1, l2, l3, l4;
    l1 = (virt >> 12) & 0x1FF;
    l2 = (virt >> 21) & 0x1FF;
    l3 = (virt >> 30) & 0x1FF;
    l4 = (virt >> 39) & 0x1FF;

    struct page_table_entry *pt = (struct page_table_entry *) boot_info->pt_ptr.l4[l4];
    struct page_table_entry *pd = (struct page_table_entry *) boot_info->pt_ptr.l3[l3];
    struct page_table_entry *pdtp = (struct page_table_entry *) boot_info->pt_ptr.l2[l2];
    struct page_table_entry *pml4 = (struct page_table_entry *) boot_info->pt_ptr.l1[l1];

    pt->present = 1;
    pt->writable = 1;
    pt->address = phys >> 12;

    pd->present = 1;
    pd->writable = 1;
    pd->address = (uint64_t) pt >> 12;

    pdtp->present = 1;
    pdtp->writable = 1;
    pdtp->address = (uint64_t) pd >> 12;

    pml4->present = 1;
    pml4->writable = 1;
    pml4->address = (uint64_t) pdtp >> 12;
}