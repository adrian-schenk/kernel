#pragma once
#include <stdint.h>

/* Intel 82540EM (e1000) register subset needed for device bring-up. */

#define E1000_CTRL      0x0000        /* Device control */
#define E1000_CTRL_RST  (1u << 26)    /* Device reset (self-clearing) */
#define E1000_RAL       0x5400        /* Receive Address Low  */
#define E1000_RAH       0x5404        /* Receive Address High */

#define E1000_RCTL      0x0100        /* Receive Control */
#define E1000_RDBAL     0x2800        /* Receive Descriptor Base Address Low */
#define E1000_RDBAH     0x2804        /* Receive Descriptor Base Address High */
#define E1000_RDLEN     0x2808        /* Receive Descriptor Length */
#define E1000_RDH       0x2810        /* Receive Descriptor Head */
#define E1000_RDT       0x2818        /* Receive Descriptor Tail */

#define E1000_TCTL      0x0400        /* Transmit Control */
#define E1000_TDBAL     0x3800        /* Transmit Descriptor Base Address Low */
#define E1000_TDBAH     0x3804        /* Transmit Descriptor Base Address High */
#define E1000_TDLEN     0x3808        /* Transmit Descriptor Length */
#define E1000_TDH       0x3810        /* Transmit Descriptor Head */
#define E1000_TDT       0x3818        /* Transmit Descriptor Tail */

/* The 82540EM exposes a 128 KB memory-mapped register space on BAR0. */
#define E1000_MMIO_SIZE 0x20000

typedef struct e1000_receive_descriptor
{
    uint64_t buffer_addr; /* Address of the receive buffer */
    uint16_t length;      /* Length of data DMAed into data buffer */
    uint16_t checksum;    /* Packet checksum */
    uint8_t status;       /* Descriptor status */
    uint8_t errors;       /* Descriptor Errors */
    uint16_t special;
} __attribute__((packed)) e1000_receive_descriptor_t;

typedef struct e1000_transmit_descriptor
{
    uint64_t buffer_addr; /* Address of the transmit buffer */
    uint16_t length;      /* Length of data DMAed into data buffer */
    uint8_t cso;          /* Checksum offset */
    uint8_t cmd;          /* Descriptor control */
    uint8_t status;       /* Descriptor status */
    uint8_t css;          /* Checksum start */
    uint16_t special;
} __attribute__((packed)) e1000_transmit_descriptor_t;

void e1000_init(void);

void e1000_network_send_tx(void* data, unsigned int len);
void e1000_network_rx_handler(void);