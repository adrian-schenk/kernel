#include "ip.h"
#include <arp/arp.h>
#include <endian.h>
#include <kmalloc.h>
#include <memory.h>
#include <printf.h>

#define IP4_MAX_PROTOCOLS 16

typedef struct {
    uint8_t protocol;
    ip4_protocol_handler_fn handler;
} ip4_protocol_entry_t;

static ip4_protocol_entry_t ip4_protocols[IP4_MAX_PROTOCOLS];
static int ip4_protocol_count = 0;
static uint16_t ip4_id = 0;

int ip4_protocol_register(uint8_t protocol, ip4_protocol_handler_fn handler)
{
    if (!handler) {
        return -1;
    }

    for (int index = 0; index < ip4_protocol_count; index++) {
        if (ip4_protocols[index].protocol == protocol) {
            ip4_protocols[index].handler = handler;
            return 0;
        }
    }

    if (ip4_protocol_count >= IP4_MAX_PROTOCOLS) {
        return -1;
    }

    ip4_protocols[ip4_protocol_count].protocol = protocol;
    ip4_protocols[ip4_protocol_count].handler = handler;
    ip4_protocol_count++;
    return 0;
}

static ip4_protocol_handler_fn ip4_find_protocol(uint8_t protocol)
{
    for (int index = 0; index < ip4_protocol_count; index++) {
        if (ip4_protocols[index].protocol == protocol) {
            return ip4_protocols[index].handler;
        }
    }

    return (void *)0;
}

void ip4_handle_packet(net_interface_t *iface, const void *packet,
                       uint16_t length)
{
    if (!iface || !packet || length < sizeof(ip_header_t)) {
        return;
    }

    const ip_header_t *header = packet;

    if ((header->version_ihl >> 4) != 4) {
        return;
    }

    uint8_t header_length = (header->version_ihl & 0x0F) * 4;
    if (header_length < sizeof(ip_header_t) || header_length > length) {
        return;
    }

    uint16_t total_length = ntohs(header->total_len);
    if (total_length < header_length || total_length > length) {
        return;
    }

    /* Fragmentation is not implemented yet. */
    if (ntohs(header->flags_frag_offset) & 0x1FFF) {
        return;
    }

    ip4_protocol_handler_fn handler = ip4_find_protocol(header->protocol);
    if (!handler) {
        return;
    }

    handler(iface, ntohl(header->source_ip), ntohl(header->dest_ip),
            (const uint8_t *)packet + header_length,
            total_length - header_length);
}

static int ip4_same_subnet(uint32_t a, uint32_t b, uint32_t netmask)
{
    return (a & netmask) == (b & netmask);
}

int ip4_send(net_interface_t *iface, uint32_t dest_ip, uint8_t protocol,
             const void *payload, uint16_t length)
{
    if (!iface || !payload) {
        return -1;
    }

    uint32_t total_length = sizeof(ip_header_t) + length;
    if (total_length > 0xFFFF) {
        return -1;
    }

    void *buffer = kmalloc((int)total_length);
    if (!buffer) {
        return -1;
    }

    ip_header_t *header = buffer;
    header->version_ihl = (4 << 4) | 5;
    header->tos = 0;
    header->total_len = htons((uint16_t)total_length);
    header->id = htons(ip4_id++);
    header->flags_frag_offset = htons(0);
    header->ttl = 64;
    header->protocol = protocol;
    header->checksum = 0;
    header->source_ip = htonl(iface->ipv4);
    header->dest_ip = htonl(dest_ip);

    memcpy((uint8_t *)buffer + sizeof(ip_header_t), payload, length);
    header->checksum = htons(net_checksum16(buffer, sizeof(ip_header_t)));

    uint32_t next_hop = dest_ip;
    if (iface->gateway &&
        !ip4_same_subnet(iface->ipv4, dest_ip, iface->netmask)) {
        next_hop = iface->gateway;
    }

    uint8_t dst_mac[NET_HWADDR_LEN_ETHERNET];
    if (arp_resolve(iface, next_hop, dst_mac) != 0) {
        kfree(buffer);
        return -1;
    }

    int result = net_link_send(iface, dst_mac, NET_PROTO_IPV4, buffer,
                               (uint16_t)total_length);
    kfree(buffer);
    return result;
}
