#include "ip.h"
#include <kernel/endian.h>
#include <printf.h>

void ip4_handle_packet(NETWORKING_ARGS, const void *packet, uint16_t length) {

    if (length < sizeof(ip_header_t)) {
        return;
    }

    const ip_header_t *header = (const ip_header_t *)packet;

    uint8_t header_length = (header->version_ihl & 0x0F) * 4;

    if ((header->version_ihl >> 4) != 4) {
        return;
    }

    if ((header->version_ihl & 0x0F) < 5) {
        return;
    }

    switch (header->protocol) {
        case 1: // ICMP
            handle_icmp_packet(dev, (const void *)((uint8_t *)packet + header_length), length - header_length);
            break;
        case 6: // TCP
            kprintf("Received TCP packet\n");
            break;
        case 17: // UDP
            kprintf("Received UDP packet\n");
            break;
        default:
            kprintf("Received unknown IP protocol: %u\n", header->protocol);
            break;
    }
    
}