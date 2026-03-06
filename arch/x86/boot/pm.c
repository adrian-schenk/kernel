#include <memlayout.h>
#include <boot_info.h>

extern uint64_t pt_ptr[];

void set_ap_boot_info() {
    struct ap_boot_info* boot_info = (struct ap_boot_info*) AP_BOOT_INFO_ADDR;

    return;
}