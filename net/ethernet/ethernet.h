#pragma once

#include <stdint.h>
#include <network.h>

typedef struct ethernet_frame_header
{
    uint8_t destination_mac[6];
    uint8_t source_mac[6];
    uint16_t ethertype;
} __attribute__((packed)) ethernet_frame_header_t;

/* Ethernet L2 implementation, registered with the core at boot. */
extern const net_link_ops_t net_ethernet_link_ops;

int net_ethernet_send(net_interface_t *iface, const uint8_t *dst,
                      uint16_t protocol, const void *payload,
                      uint16_t length);
void net_ethernet_receive(net_interface_t *iface, const void *frame,
                          uint16_t length);
