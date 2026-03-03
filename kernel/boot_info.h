#include <stdint.h>

struct ap_boot_info {
    uint64_t l4_table_phys;   // Physical address of CR3 / PML4
    uint64_t kernel_entry;    // 64-bit entry point for AP
    uint64_t stack_ptr;       // Initial stack for AP
    uint32_t cpu_id;          // AP ID (optional)
    uint32_t flags;           // e.g., started flag
};