#include <memlayout.h>
#include <boot_info.h>

extern uint64_t page_table_l4;

void set_ap_boot_info() {
    struct ap_boot_info* boot_info = (struct ap_boot_info*) AP_BOOT_INFO_ADDR;

    //boot_info->l4_table_phys = page_table_l4;

    

    
}