#pragma once
#include <stdint.h>

static inline void mmio_writell(uint64_t addr, uint64_t value) {
    *(volatile uint64_t*)addr = value;
}

static inline uint64_t mmio_readll(uint64_t addr) {
    return *(volatile uint64_t*)addr;
}

static inline void mmio_writel(uint64_t addr, uint32_t value) {
    *(volatile uint32_t*)addr = value;
}

static inline uint32_t mmio_readl(uint64_t addr) {
    return *(volatile uint32_t*)addr;
}

static inline void mmio_writew(uint64_t addr, uint16_t value) {
    *(volatile uint16_t*)addr = value;
}

static inline uint16_t mmio_readw(uint64_t addr) {
    return *(volatile uint16_t*)addr;
}

static inline void mmio_writeb(uint64_t addr, uint8_t value) {
    *(volatile uint8_t*)addr = value;
}

static inline uint8_t mmio_readb(uint64_t addr) {
    return *(volatile uint8_t*)addr;
}