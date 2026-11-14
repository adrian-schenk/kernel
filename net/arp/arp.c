#include <arp/arp.h>
#include <endian.h>
#include <memory.h>
#include <printf.h>

#define ARP_CACHE_SIZE 16

typedef struct {
    net_interface_t *iface;
    uint32_t ip;
    uint8_t mac[NET_HWADDR_LEN_ETHERNET];
    uint8_t valid;
} arp_cache_entry_t;

static arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];
static int arp_cache_next = 0;

static arp_cache_entry_t *arp_cache_find(net_interface_t *iface, uint32_t ip)
{
    for (int index = 0; index < ARP_CACHE_SIZE; index++) {
        arp_cache_entry_t *entry = &arp_cache[index];

        if (entry->valid && entry->iface == iface && entry->ip == ip) {
            return entry;
        }
    }

    return (void *)0;
}

static void arp_cache_store(net_interface_t *iface, uint32_t ip,
                            const uint8_t mac[NET_HWADDR_LEN_ETHERNET])
{
    if (!iface || ip == 0) {
        return;
    }

    arp_cache_entry_t *entry = arp_cache_find(iface, ip);

    if (!entry) {
        for (int index = 0; index < ARP_CACHE_SIZE; index++) {
            if (!arp_cache[index].valid) {
                entry = &arp_cache[index];
                break;
            }
        }
    }

    if (!entry) {
        entry = &arp_cache[arp_cache_next];
        arp_cache_next = (arp_cache_next + 1) % ARP_CACHE_SIZE;
    }

    entry->iface = iface;
    entry->ip = ip;
    memcpy(entry->mac, mac, NET_HWADDR_LEN_ETHERNET);
    entry->valid = 1;
}

static int arp_send(net_interface_t *iface, const uint8_t *dst_mac,
                    const uint8_t *target_mac, uint32_t target_ip,
                    uint16_t opcode)
{
    arp_packet_t packet;

    memset(&packet, 0, sizeof(packet));
    packet.hardware_type = htons(ARP_HARDWARE_ETHERNET);
    packet.protocol_type = htons(NET_PROTO_IPV4);
    packet.hardware_size = NET_HWADDR_LEN_ETHERNET;
    packet.protocol_size = 4;
    packet.opcode = htons(opcode);

    memcpy(packet.sender_mac, iface->hwaddr, NET_HWADDR_LEN_ETHERNET);
    net_ipv4_to_bytes(iface->ipv4, packet.sender_ip);

    if (target_mac) {
        memcpy(packet.target_mac, target_mac, NET_HWADDR_LEN_ETHERNET);
    }

    net_ipv4_to_bytes(target_ip, packet.target_ip);

    return net_link_send(iface, dst_mac, NET_PROTO_ARP, &packet,
                         sizeof(packet));
}

int arp_send_request(net_interface_t *iface, uint32_t target_ip)
{
    static const uint8_t broadcast[NET_HWADDR_LEN_ETHERNET] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };

    if (!iface) {
        return -1;
    }

    return arp_send(iface, broadcast, (void *)0, target_ip, ARP_OPCODE_REQUEST);
}

int arp_resolve(net_interface_t *iface, uint32_t ip, uint8_t out_mac[6])
{
    if (!iface || !out_mac) {
        return -1;
    }

    /* Resolving our own address never needs a request. */
    if (ip == iface->ipv4) {
        memcpy(out_mac, iface->hwaddr, NET_HWADDR_LEN_ETHERNET);
        return 0;
    }

    arp_cache_entry_t *entry = arp_cache_find(iface, ip);

    if (entry) {
        memcpy(out_mac, entry->mac, NET_HWADDR_LEN_ETHERNET);
        return 0;
    }

    arp_send_request(iface, ip);
    return -1;
}

void arp_handle_packet(net_interface_t *iface, const void *packet,
                       uint16_t length)
{
    if (!iface || !packet || length < sizeof(arp_packet_t)) {
        return;
    }

    const arp_packet_t *arp = packet;

    if (ntohs(arp->hardware_type) != ARP_HARDWARE_ETHERNET ||
        ntohs(arp->protocol_type) != NET_PROTO_IPV4 ||
        arp->hardware_size != NET_HWADDR_LEN_ETHERNET ||
        arp->protocol_size != 4) {
        return;
    }

    uint32_t sender_ip = net_ipv4_from_bytes(arp->sender_ip);
    uint32_t target_ip = net_ipv4_from_bytes(arp->target_ip);
    uint16_t opcode = ntohs(arp->opcode);

    /* Every ARP packet is a chance to learn a neighbour. */
    arp_cache_store(iface, sender_ip, arp->sender_mac);

    if (opcode == ARP_OPCODE_REQUEST && target_ip == iface->ipv4 &&
        sender_ip != iface->ipv4) {
        arp_send(iface, arp->sender_mac, arp->sender_mac, sender_ip,
                 ARP_OPCODE_REPLY);
    }
}
