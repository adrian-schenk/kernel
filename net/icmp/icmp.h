#pragma once
#include <network.h>
#include <ip/ip.h>

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t rest_of_header;
} icmp_header_t;

void handle_icmp_packet(NETWORKING_ARGS, const void *icmp_packet, uint16_t length);