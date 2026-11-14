#pragma once

#include <stdint.h>
#include <network.h>

#define ICMP_TYPE_ECHO_REPLY   0
#define ICMP_TYPE_ECHO_REQUEST 8

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t rest_of_header;
} icmp_header_t;

void icmp_handle_packet(net_interface_t *iface, uint32_t source_ip,
                        uint32_t dest_ip, const void *icmp_packet,
                        uint16_t length);
