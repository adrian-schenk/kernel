#include "icmp.h"
#include <printf.h>
#include <ip/ip.h>
#include <ethernet/ethernet.h>


void handle_icmp_packet(NETWORKING_ARGS, const void *icmp_packet, uint16_t length) {
    if (length < sizeof(icmp_header_t)) {
        return;
    }

    const icmp_header_t *header = (const icmp_header_t *)icmp_packet;

    kprintf("Received ICMP packet: type=%u, code=%u\n", header->type, header->code);

    switch (header->type) {
        case 0x08:
            kprintf("ICMP Echo Request received.\n");
            icmp_header_t reply = {
                .type = 0x00,
                .code = 0x00,
                .checksum = 0,
                .rest_of_header = 0,
            };

            break;
        case 0x00:
            kprintf("ICMP Echo Reply received.\n");
            // Handle echo reply
            break;
        default:
            kprintf("Unhandled ICMP type: %u\n", header->type);
            break;
    }
}