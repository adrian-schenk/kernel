#pragma once
#include <stdint.h>

// x86 is little endian

#define swap_bytes_16(x) (((x) << 8) | ((x) >> 8))
#define swap_bytes_32(x) (((x) << 24) | (((x) << 8) & 0x00FF0000) | (((x) >> 8) & 0x0000FF00)) | ((x) >> 24)
#define swap_bytes_64(x) (((x) << 56) | (((x) << 40) & 0x00FF000000000000ULL) | (((x) << 24) & 0x0000FF0000000000ULL) | (((x) << 8)  & 0x000000FF00000000ULL) | (((x) >> 8)  & 0x00000000FF000000ULL) | (((x) >> 24) & 0x0000000000FF0000ULL) | (((x) >> 40) & 0x000000000000FF00ULL) | ((x) >> 56));


static uint8_t htonb(uint8_t x) {
    return x;
}

static uint16_t htons(uint16_t x) {
    return swap_bytes_16(x);
}

static uint32_t htonl(uint32_t x) {
    return swap_bytes_32(x);
}

static uint64_t htonll(uint64_t x) {
    return swap_bytes_64(x);
}

static uint8_t ntohb(uint8_t x) {
    return x;
}

static uint16_t ntohs(uint16_t x) {
    return swap_bytes_16(x);
}

static uint32_t ntohl(uint32_t x) {
    return swap_bytes_32(x);
}

static uint64_t ntohll(uint64_t x) {
    return swap_bytes_64(x);
}