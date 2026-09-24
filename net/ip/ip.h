#pragma once
#include <stdint.h>

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

void ip4_handle_packet(const void *packet, uint16_t length);