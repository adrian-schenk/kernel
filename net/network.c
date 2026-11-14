#include <network.h>
#include <ethernet/ethernet.h>
#include <arp/arp.h>
#include <ip/ip.h>
#include <icmp/icmp.h>
#include <e1000.h>
#include <printf.h>
#include <memory.h>

/*
 * Default configuration applied by the core when an interface is created.
 * There is deliberately no ioctl/ifconfig layer yet.
 */
#define NET_DEFAULT_IPV4    NET_IPV4(10, 8, 0, 1)
#define NET_DEFAULT_NETMASK NET_IPV4(255, 255, 255, 0)

static net_device_t *network_devices[NET_MAX_DEVICES];
static int network_device_count = 0;

static net_interface_t network_interfaces[NET_MAX_INTERFACES];
static net_interface_t *network_interface_list[NET_MAX_INTERFACES];
static int network_interface_count = 0;

static const net_link_ops_t *network_links[NET_MAX_LINKS];
static int network_link_count = 0;

typedef struct {
    uint16_t protocol;
    net_rx_handler_fn handler;
} net_protocol_entry_t;

static net_protocol_entry_t network_protocols[NET_MAX_PROTOCOLS];
static int network_protocol_count = 0;

static net_device_t *primary_network_device = (void *)0;

/* ---- device registry ---------------------------------------------------- */

int net_device_register(net_device_t *device)
{
    if (!device || !device->ops || !device->ops->send || !device->ops->poll) {
        return -1;
    }

    if (network_device_count >= NET_MAX_DEVICES) {
        return -1;
    }

    if (device->hwaddr_len == 0 && device->link) {
        device->hwaddr_len = device->link->address_length;
    }

    device->interface = (void *)0;
    network_devices[network_device_count++] = device;

    if (!primary_network_device) {
        primary_network_device = device;
    }

    return 0;
}

int net_device_count(void)
{
    return network_device_count;
}

net_device_t *net_device_get(int index)
{
    if (index < 0 || index >= network_device_count) {
        return (void *)0;
    }

    return network_devices[index];
}

/* ---- interface registry ------------------------------------------------- */

static void interface_set_default_name(net_interface_t *iface, int index)
{
    int pos = 0;

    iface->name[pos++] = 'e';
    iface->name[pos++] = 't';
    iface->name[pos++] = 'h';

    if (index >= 10) {
        iface->name[pos++] = '0' + ((index / 10) % 10);
    }

    iface->name[pos++] = '0' + (index % 10);
    iface->name[pos] = '\0';
}

net_interface_t *net_interface_attach(net_device_t *device)
{
    if (!device || !device->link) {
        return (void *)0;
    }

    if (device->interface) {
        return device->interface;
    }

    if (network_interface_count >= NET_MAX_INTERFACES) {
        return (void *)0;
    }

    net_interface_t *iface = &network_interfaces[network_interface_count];
    memset(iface, 0, sizeof(*iface));

    interface_set_default_name(iface, network_interface_count);
    iface->device = device;
    iface->link = device->link;
    iface->hwaddr_len = device->hwaddr_len;

    if (iface->hwaddr_len > NET_HWADDR_MAX) {
        iface->hwaddr_len = NET_HWADDR_MAX;
    }

    memcpy(iface->hwaddr, device->hwaddr, iface->hwaddr_len);
    device->interface = iface;

    network_interface_list[network_interface_count++] = iface;
    return iface;
}

int net_interface_count(void)
{
    return network_interface_count;
}

net_interface_t *net_interface_get(int index)
{
    if (index < 0 || index >= network_interface_count) {
        return (void *)0;
    }

    return network_interface_list[index];
}

net_interface_t *net_interface_primary(void)
{
    return net_interface_get(0);
}

net_interface_t *net_interface_by_device(net_device_t *device)
{
    if (!device) {
        return (void *)0;
    }

    return device->interface;
}

int net_interface_set_ipv4(net_interface_t *iface, uint32_t address,
                           uint32_t netmask, uint32_t gateway)
{
    if (!iface) {
        return -1;
    }

    iface->ipv4 = address;
    iface->netmask = netmask;
    iface->gateway = gateway;
    return 0;
}

/* ---- L2 protocol registry / data path ----------------------------------- */

int net_link_register(const net_link_ops_t *ops)
{
    if (!ops || !ops->name || !ops->send || !ops->receive) {
        return -1;
    }

    for (int index = 0; index < network_link_count; index++) {
        if (network_links[index] == ops) {
            return 0;
        }
    }

    if (network_link_count >= NET_MAX_LINKS) {
        return -1;
    }

    network_links[network_link_count++] = ops;
    return 0;
}

const net_link_ops_t *net_link_get(const char *name)
{
    if (!name) {
        return (void *)0;
    }

    for (int index = 0; index < network_link_count; index++) {
        const char *candidate = network_links[index]->name;
        int match = 1;

        while (*name && *candidate) {
            if (*name++ != *candidate++) {
                match = 0;
                break;
            }
        }

        if (match && *name == '\0' && *candidate == '\0') {
            return network_links[index];
        }
    }

    return (void *)0;
}

int net_link_send(net_interface_t *iface, const uint8_t *dst,
                  uint16_t protocol, const void *payload, uint16_t length)
{
    if (!iface || !iface->link || !iface->link->send || !dst || !payload) {
        return -1;
    }

    return iface->link->send(iface, dst, protocol, payload, length);
}

void net_link_deliver(net_interface_t *iface, uint16_t protocol,
                      const void *payload, uint16_t length)
{
    for (int index = 0; index < network_protocol_count; index++) {
        if (network_protocols[index].protocol == protocol) {
            network_protocols[index].handler(iface, payload, length);
            return;
        }
    }
}

/* ---- L3 protocol registry ----------------------------------------------- */

int net_protocol_register(uint16_t protocol, net_rx_handler_fn handler)
{
    if (!handler) {
        return -1;
    }

    for (int index = 0; index < network_protocol_count; index++) {
        if (network_protocols[index].protocol == protocol) {
            network_protocols[index].handler = handler;
            return 0;
        }
    }

    if (network_protocol_count >= NET_MAX_PROTOCOLS) {
        return -1;
    }

    network_protocols[network_protocol_count].protocol = protocol;
    network_protocols[network_protocol_count].handler = handler;
    network_protocol_count++;
    return 0;
}

/* ---- driver-facing helpers ---------------------------------------------- */

int network_send(network_device_t *dev, const void *data, uint16_t length)
{
    if (!dev || !dev->ops || !dev->ops->send) {
        return -1;
    }

    return dev->ops->send(dev, data, length);
}

int network_poll(void)
{
    int processed = 0;

    for (int index = 0; index < network_device_count; index++) {
        processed += network_devices[index]->ops->poll(network_devices[index]);
    }

    return processed;
}

void network_handle_frame(network_device_t *device, void *frame,
                          uint16_t length)
{
    if (!device || !device->interface || !device->interface->link) {
        return;
    }

    device->interface->link->receive(device->interface, frame, length);
}

void network_rx_worker(void)
{
    for (;;) {
        if (network_poll() == 0) {
            __asm__ volatile ("hlt");
        }
    }
}

/* ---- core initialization ------------------------------------------------ */

int network_init(void)
{
    /*
     * Register the link layers and L3 protocols that are compiled in. Adding
     * another L2 protocol or NIC only requires registering it here (or from
     * its own init hook) - the layers below/above stay untouched.
     */
    net_link_register(&net_ethernet_link_ops);
    net_protocol_register(NET_PROTO_ARP, arp_handle_packet);
    net_protocol_register(NET_PROTO_IPV4, ip4_handle_packet);

    /* L3 protocol numbers. TCP (6) and UDP (17) will register here later. */
    ip4_protocol_register(IP4_PROTOCOL_ICMP, icmp_handle_packet);

    static network_device_t e1000_device = {
        .name = "e1000",
    };

    if (e1000_init(&e1000_device) != 0) {
        kprintf("network: no supported NIC initialized\n");
        return -1;
    }

    e1000_device.link = net_link_get("ethernet");
    if (net_device_register(&e1000_device) != 0) {
        kprintf("network: failed to register device %s\n", e1000_device.name);
        return -1;
    }

    net_interface_t *iface = net_interface_attach(&e1000_device);
    if (!iface) {
        kprintf("network: failed to create interface for %s\n",
                e1000_device.name);
        return -1;
    }

    /*
     * Single place where an interface gets its logical address for now.
     * A future configuration layer can replace this without changing the
     * networking core.
     */
    net_interface_set_ipv4(iface, NET_DEFAULT_IPV4, NET_DEFAULT_NETMASK, 0);

    kprintf("network: %s up, %s is 10.8.0.1/24\n",
            iface->name, e1000_device.name);
    return 0;
}

/* ---- backwards-compatible accessors ------------------------------------- */

network_device_t *network_primary_device(void)
{
    return primary_network_device;
}

const uint8_t *network_primary_mac(void)
{
    net_interface_t *iface = net_interface_primary();

    if (!iface) {
        return (void *)0;
    }

    return iface->hwaddr;
}

const uint8_t *network_primary_ipv4(void)
{
    static uint8_t ipv4[4];
    net_interface_t *iface = net_interface_primary();

    if (!iface) {
        return (void *)0;
    }

    net_ipv4_to_bytes(iface->ipv4, ipv4);
    return ipv4;
}

/* ---- misc helpers ------------------------------------------------------- */

void net_mac_to_string(const uint8_t mac[6], char out[18])
{
    static const char hex_digits[] = "0123456789ABCDEF";
    int pos = 0;

    for (int index = 0; index < 6; index++) {
        out[pos++] = hex_digits[mac[index] >> 4];
        out[pos++] = hex_digits[mac[index] & 0xF];

        if (index < 5) {
            out[pos++] = ':';
        }
    }

    out[pos] = '\0';
}

void net_ipv4_to_bytes(uint32_t address, uint8_t out[4])
{
    out[0] = (uint8_t)(address >> 24);
    out[1] = (uint8_t)(address >> 16);
    out[2] = (uint8_t)(address >> 8);
    out[3] = (uint8_t)address;
}

uint32_t net_ipv4_from_bytes(const uint8_t in[4])
{
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) |
           ((uint32_t)in[2] << 8) | (uint32_t)in[3];
}

uint16_t net_checksum16(const void *data, uint16_t length)
{
    const uint8_t *bytes = data;
    uint32_t sum = 0;

    while (length > 1) {
        sum += ((uint16_t)bytes[0] << 8) | bytes[1];
        bytes += 2;
        length -= 2;
    }

    if (length != 0) {
        sum += (uint16_t)bytes[0] << 8;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)~sum;
}
