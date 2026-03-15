#include "page.h"
#include "memlayout.h"

int page_offset = 0;

struct page_table _l1 __attribute__((aligned(4096))) = {0};
struct page_table _l2 __attribute__((aligned(4096))) = {0};
struct page_table _l3 __attribute__((aligned(4096))) = {0};
struct page_table _l4 __attribute__((aligned(4096))) = {0};

struct page_table kernel_pml4 __attribute__((aligned(4096))) = {0};

int pt_map_page(struct page_table *pml4,
                uint64_t virt,
                uint64_t phys,
                uint64_t flags) {
    uint16_t i_pt   = IDX_PT(virt);
    uint16_t i_pd   = IDX_PD(virt);
    uint16_t i_pdpt = IDX_PDPT(virt);
    uint16_t i_pml4 = IDX_PML4(virt);

    struct page_table *pdpt;
    struct page_table *pd;
    struct page_table *pt;

    pdpt = get_next_table(pml4, i_pml4);
    pd   = get_next_table(pdpt, i_pdpt);
    pt   = get_next_table(pd, i_pd);

    pt->entries[i_pt].present = flags & PAGE_PRESENT ? 1 : 0;
    pt->entries[i_pt].writable = flags & PAGE_WRITABLE ? 1 : 0;
    pt->entries[i_pt].user = flags & PAGE_USER ? 1 : 0;
    pt->entries[i_pt].pwt = flags & PAGE_PWT ? 1 : 0;
    pt->entries[i_pt].pcd = flags & PAGE_PCD ? 1 : 0;
    pt->entries[i_pt].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;

    pt->entries[i_pt].address  = phys >> 12;

    asm volatile("invlpg (%0)" ::"r"(virt) : "memory");
    
    return 0;
}

int pt_map_page_huge(struct page_table *pml4,
                     uint64_t virt,
                     uint64_t phys,
                     uint64_t flags) {
    uint16_t i_pd   = IDX_PD(virt);
    uint16_t i_pdpt = IDX_PDPT(virt);
    uint16_t i_pml4 = IDX_PML4(virt);

    struct page_table *pdpt;
    struct page_table *pd;

    pdpt = get_next_table(pml4, i_pml4);
    pd   = get_next_table(pdpt, i_pdpt);

    pd->entries[i_pd].present = flags & PAGE_PRESENT ? 1 : 0;
    pd->entries[i_pd].writable = flags & PAGE_WRITABLE ? 1 : 0;
    pd->entries[i_pd].user = flags & PAGE_USER ? 1 : 0;
    pd->entries[i_pd].pwt = flags & PAGE_PWT ? 1 : 0;
    pd->entries[i_pd].pcd = flags & PAGE_PCD ? 1 : 0;
    pd->entries[i_pd].no_execute = flags & PAGE_NO_EXECUTE ? 1 : 0;
    pd->entries[i_pd].present   = 1;
    pd->entries[i_pd].writable  = (flags & PAGE_WRITABLE) ? 1 : 0;
    pd->entries[i_pd].huge_page = 1;

    pd->entries[i_pd].address = phys >> 21;

    asm volatile("invlpg (%0)" ::"r"(virt) : "memory");


    return 0;
}

static struct page_table *get_next_table(struct page_table *current, uint16_t index) {
    if (!current->entries[index].present) {

        struct page_table *new_table =
            (struct page_table*)((uint8_t*)PAGE_TABLE_REGION + page_offset);

        page_offset += PAGE_SIZE;

        pt_clear_page(new_table);

        current->entries[index].present  = 1;
        current->entries[index].writable = 1;
        current->entries[index].address  = ((uint64_t)new_table) >> 12;
    }

    return (struct page_table*)
           (current->entries[index].address << 12);
}

void pt_clear_page(void* p) {
    uint64_t* t = p;
    for(int i=0;i<512;i++)
        t[i] = 0;
}

void pt_setup() {

    for (int i = 0; i < 0x40000000; i += 0x200000) {
        pt_map_page_huge(&kernel_pml4, i, i, PAGE_PRESENT | PAGE_WRITABLE);
    }

    asm volatile (
        "mov %0, %%cr3"
        :        
        : "r" (&kernel_pml4)
        : "memory"
    );

    boot_info->pt_ptr.l1 = &kernel_pml4;
}