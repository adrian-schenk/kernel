#pragma once
#include <stdint.h>

/* Intel 82540EM (e1000) register subset needed for device bring-up. */

#define E1000_CTRL      0x0000        /* Device control */
#define E1000_CTRL_RST  (1u << 26)    /* Device reset (self-clearing) */
#define E1000_RAL       0x5400        /* Receive Address Low  */
#define E1000_RAH       0x5404        /* Receive Address High */

/* The 82540EM exposes a 128 KB memory-mapped register space on BAR0. */
#define E1000_MMIO_SIZE 0x20000

void e1000_init(void);
