#pragma once

#include <stdint.h>
#include <network.h>

#define ARP_HARDWARE_ETHERNET 1
#define ARP_OPCODE_REQUEST 1
#define ARP_OPCODE_REPLY   2

typedef struct arp_packet
{
    uint16_t hardware_type;
    uint16_t protocol_type;
    uint8_t hardware_size;
    uint8_t protocol_size;
    uint16_t opcode;
    uint8_t sender_mac[NET_HWADDR_LEN_ETHERNET];
    uint8_t sender_ip[4];
    uint8_t target_mac[NET_HWADDR_LEN_ETHERNET];
    uint8_t target_ip[4];
} __attribute__((packed)) arp_packet_t;

/* L3 handler for NET_PROTO_ARP. */
void arp_handle_packet(net_interface_t *iface, const void *packet,
                       uint16_t length);

/* Resolve an IPv4 next hop to an L2 address. Sends an ARP request and returns
 * -1 when the entry is not cached yet, so callers can drop/retry the packet. */
int arp_resolve(net_interface_t *iface, uint32_t ip, uint8_t out_mac[6]);
int arp_send_request(net_interface_t *iface, uint32_t target_ip);
