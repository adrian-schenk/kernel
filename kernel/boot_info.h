#include <stdint.h>

extern struct ap_boot_info* boot_info;

struct pt_ptr {
    uint64_t* l1; // Physical address of CR3
    uint64_t* l2; // Physical address of PDTP
    uint64_t* l3; // Physical address of PD
    uint64_t* l4; // Physical address of PT
} __attribute__((packed));

struct ap_boot_info {
    struct pt_ptr pt_ptr;     // Physical addresses of page tables 
    uint64_t kernel_entry;    // 64-bit entry point for AP
    uint64_t stack_ptr;       // Initial stack for AP
    uint32_t cpu_id;          // AP ID (optional)
    uint32_t flags;           // e.g., started flag
} __attribute__((packed));

extern unsigned char tmp_stack[4096];