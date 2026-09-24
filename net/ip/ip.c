#include "ip.h"
#include <kernel/endian.h>
#include <printf.h>

void ip4_handle_packet(const void *packet, uint16_t length) {

    if (length < sizeof(ip_header_t)) {
        return;
    }

    const ip_header_t *header = (const ip_header_t *)packet;

    if ((header->version_ihl >> 4) != 4) {
        return;
    }

    if ((header->version_ihl & 0x0F) < 5) {
        return;
    }

    kprintf("%x\n", header->protocol);
}