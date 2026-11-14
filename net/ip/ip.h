#pragma once

#include <stdint.h>
#include <network.h>

#define IP4_PROTOCOL_ICMP 1
#define IP4_PROTOCOL_TCP  6
#define IP4_PROTOCOL_UDP  17

typedef struct {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t flags_frag_offset;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t source_ip;
    uint32_t dest_ip;
    uint8_t  options[];
} ip_header_t;

typedef void (*ip4_protocol_handler_fn)(net_interface_t *iface,
                                        uint32_t source_ip,
                                        uint32_t dest_ip,
                                        const void *payload,
                                        uint16_t length);

/* L3 handler for NET_PROTO_IPV4. */
void ip4_handle_packet(net_interface_t *iface, const void *packet,
                       uint16_t length);

/* Register an upper-layer protocol (ICMP, TCP, UDP, ...). */
int ip4_protocol_register(uint8_t protocol, ip4_protocol_handler_fn handler);

/* Build an IPv4 packet, resolve the next hop through ARP and hand the frame
 * to the interface's L2 implementation. */
int ip4_send(net_interface_t *iface, uint32_t dest_ip, uint8_t protocol,
             const void *payload, uint16_t length);
