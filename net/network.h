#pragma once

#include <stdint.h>

/*
 * Core networking object model
 * ---------------------------
 *   net_device      - one physical NIC (e1000, virtio-net, ...)
 *   net_link_ops    - an L2 protocol (Ethernet, ...)
 *   net_interface   - a logical interface bound to a device + link layer,
 *                     carrying its L2 address and L3 (IPv4) configuration
 *   net_rx_handler  - an L3 protocol (IPv4, ARP, ...) registered against a
 *                     link-independent protocol number
 *
 * Drivers only talk to net_device_* APIs. Link layers only build/parse their
 * own headers and call net_link_send()/net_link_deliver(). L3 protocols never
 * know which NIC or L2 implementation is underneath them.
 */

#define NET_MAX_DEVICES 8
#define NET_MAX_INTERFACES 8
#define NET_MAX_LINKS 8
#define NET_MAX_PROTOCOLS 16
#define NET_HWADDR_MAX 8

#define NET_HWADDR_LEN_ETHERNET 6

/* Link-independent protocol identifiers. Ethernet happens to use the same
 * values as its ethertype field; other L2 implementations can map their own
 * protocol identifiers onto these values before calling net_link_deliver(). */
#define NET_PROTO_IPV4 0x0800
#define NET_PROTO_ARP  0x0806

#define NET_IPV4(a, b, c, d) \
    ((((uint32_t)(a)) << 24) | (((uint32_t)(b)) << 16) | \
     (((uint32_t)(c)) << 8) | ((uint32_t)(d)))

typedef struct net_device net_device_t;
typedef struct net_interface net_interface_t;
typedef struct net_link_ops net_link_ops_t;

/* Kept as an alias for the old driver-facing name. */
typedef net_device_t network_device_t;

typedef int (*net_device_send_fn)(net_device_t *device, const void *data,
                                  uint16_t length);
typedef int (*net_device_poll_fn)(net_device_t *device);

typedef struct net_device_ops {
    /* Transmit one complete L2 frame. */
    net_device_send_fn send;
    /* Poll the NIC for received frames. */
    net_device_poll_fn poll;
} net_device_ops_t;

/* Kept as an alias for the old driver-facing name. */
typedef net_device_ops_t network_device_ops_t;

struct net_device {
    const char *name;
    uint8_t hwaddr[NET_HWADDR_MAX];
    uint8_t hwaddr_len;
    void *driver_data;
    const net_device_ops_t *ops;
    const net_link_ops_t *link;
    net_interface_t *interface;
};

/*
 * L2 protocol interface. A link implementation owns its frame format and maps
 * its protocol field onto the link-independent NET_PROTO_* values.
 */
typedef int (*net_link_send_fn)(net_interface_t *iface, const uint8_t *dst,
                                uint16_t protocol, const void *payload,
                                uint16_t length);
typedef void (*net_link_receive_fn)(net_interface_t *iface, const void *frame,
                                    uint16_t length);

struct net_link_ops {
    const char *name;
    uint8_t address_length;
    uint16_t header_length;
    net_link_send_fn send;
    net_link_receive_fn receive;
};

/*
 * L3 protocol receive handler. The payload starts after the L2 header and is
 * only valid for the duration of the call.
 */
typedef void (*net_rx_handler_fn)(net_interface_t *iface, const void *payload,
                                  uint16_t length);

struct net_interface {
    char name[8];
    net_device_t *device;
    const net_link_ops_t *link;
    uint8_t hwaddr[NET_HWADDR_MAX];
    uint8_t hwaddr_len;
    uint32_t ipv4;    /* host byte order */
    uint32_t netmask; /* host byte order */
    uint32_t gateway; /* host byte order, 0 when unset */
};

/* ---- device registry ---------------------------------------------------- */

int net_device_register(net_device_t *device);
int net_device_count(void);
net_device_t *net_device_get(int index);

/* ---- interface registry ------------------------------------------------- */

net_interface_t *net_interface_attach(net_device_t *device);
int net_interface_count(void);
net_interface_t *net_interface_get(int index);
net_interface_t *net_interface_primary(void);
net_interface_t *net_interface_by_device(net_device_t *device);
int net_interface_set_ipv4(net_interface_t *iface, uint32_t address,
                           uint32_t netmask, uint32_t gateway);

/* ---- L2 protocol registry / data path ----------------------------------- */

int net_link_register(const net_link_ops_t *ops);
const net_link_ops_t *net_link_get(const char *name);
int net_link_send(net_interface_t *iface, const uint8_t *dst,
                  uint16_t protocol, const void *payload, uint16_t length);
void net_link_deliver(net_interface_t *iface, uint16_t protocol,
                      const void *payload, uint16_t length);

/* ---- L3 protocol registry ----------------------------------------------- */

int net_protocol_register(uint16_t protocol, net_rx_handler_fn handler);

/* ---- driver-facing helpers ---------------------------------------------- */

void network_handle_frame(network_device_t *device, void *frame,
                          uint16_t length);
int network_send(network_device_t *dev, const void *data, uint16_t length);
int network_poll(void);
void network_rx_worker(void);
int network_init(void);

/* Backwards-compatible convenience accessors. Prefer the interface API. */
network_device_t *network_primary_device(void);
const uint8_t *network_primary_mac(void);
const uint8_t *network_primary_ipv4(void);

/* ---- misc helpers ------------------------------------------------------- */

void net_mac_to_string(const uint8_t mac[6], char out[18]);
void net_ipv4_to_bytes(uint32_t address, uint8_t out[4]);
uint32_t net_ipv4_from_bytes(const uint8_t in[4]);
uint16_t net_checksum16(const void *data, uint16_t length);
