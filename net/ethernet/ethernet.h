#pragma once
#include <stdint.h>
#include <printf.h>
#include <e1000.h>

static const char hex_digits[] = "0123456789ABCDEF";

typedef struct ethernet_frame_header
{
    uint8_t destination_mac[6];
    uint8_t source_mac[6];
    uint16_t ethertype;
} __attribute__((packed)) ethernet_frame_header_t;

void handle_ethernet_frame(ethernet_frame_header_t *frame, uint16_t length);


static void mac_to_string(const uint8_t mac[6], char out[18])
{
    int j = 0;
    for (int i = 0; i < 6; i++)
    {
        out[j++] = hex_digits[mac[i] >> 4];
        out[j++] = hex_digits[mac[i] & 0xF];
        if (i < 5)
            out[j++] = ':';
    }
    out[j] = '\0';
}