#include <memlayout.h>
#include <boot_info.h>
#include <vesa.h>

extern uint64_t pt_ptr[];
extern VbeModeInfoBlock video_info;
extern VbeInfoBlock vbe_info;

struct ap_boot_info* boot_info = (struct ap_boot_info*) AP_BOOT_INFO_ADDR;

void set_ap_boot_info() {
    boot_info->vesa_info = &video_info;
    return;
}