#include "icmp.h"
#include <endian.h>
#include <ip/ip.h>
#include <kmalloc.h>
#include <memory.h>
#include <printf.h>

void icmp_handle_packet(net_interface_t *iface, uint32_t source_ip,
                        uint32_t dest_ip, const void *icmp_packet,
                        uint16_t length)
{
    (void)dest_ip;

    if (!iface || !icmp_packet || length < sizeof(icmp_header_t)) {
        return;
    }

    const icmp_header_t *header = icmp_packet;

    kprintf("Received ICMP packet: type=%u, code=%u\n",
            header->type, header->code);

    switch (header->type) {
        case ICMP_TYPE_ECHO_REQUEST: {
            uint8_t *reply = kmalloc(length);
            if (!reply) {
                return;
            }

            memcpy(reply, icmp_packet, length);

            icmp_header_t *reply_header = (icmp_header_t *)reply;
            reply_header->type = ICMP_TYPE_ECHO_REPLY;
            reply_header->code = 0;
            reply_header->checksum = 0;
            reply_header->checksum = htons(net_checksum16(reply, length));

            ip4_send(iface, source_ip, IP4_PROTOCOL_ICMP, reply, length);
            kfree(reply);
            break;
        }
        case ICMP_TYPE_ECHO_REPLY:
            break;
        default:
            kprintf("Unhandled ICMP type: %u\n", header->type);
            break;
    }
}
