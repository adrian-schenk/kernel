#include <ethernet/ethernet.h>
#include <endian.h>
#include <kmalloc.h>
#include <memory.h>
#include <printf.h>

void net_ethernet_receive(net_interface_t *iface, const void *frame,
                          uint16_t length)
{
    if (!iface || !frame || length < sizeof(ethernet_frame_header_t)) {
        return;
    }

    const ethernet_frame_header_t *header = frame;
    uint16_t ethertype = ntohs(header->ethertype);
    const void *payload = (const uint8_t *)frame + sizeof(*header);
    uint16_t payload_length = length - sizeof(*header);

    char src_mac[18], dst_mac[18];
    net_mac_to_string(header->source_mac, src_mac);
    net_mac_to_string(header->destination_mac, dst_mac);

    switch (ethertype) {
        case NET_PROTO_IPV4:
            kprintf("Received IPv4 packet: Src MAC: %s, Dst MAC: %s, Length: %u\n",
                    src_mac, dst_mac, length);
            break;
        case NET_PROTO_ARP:
            kprintf("Received ARP packet: Src MAC: %s, Dst MAC: %s, Length: %u\n",
                    src_mac, dst_mac, length);
            break;
        default:
            kprintf("Received unknown Ethernet frame: Ethertype: 0x%04X, "
                    "Src MAC: %s, Dst MAC: %s, Length: %u\n",
                    ethertype, src_mac, dst_mac, length);
            break;
    }

    net_link_deliver(iface, ethertype, payload, payload_length);
}

int net_ethernet_send(net_interface_t *iface, const uint8_t *dst,
                      uint16_t protocol, const void *payload,
                      uint16_t length)
{
    if (!iface || !iface->device || !iface->device->ops ||
        !iface->device->ops->send || !dst || !payload ||
        iface->hwaddr_len != NET_HWADDR_LEN_ETHERNET) {
        return -1;
    }

    uint16_t frame_length = sizeof(ethernet_frame_header_t) + length;
    void *buffer = kmalloc(frame_length);
    if (!buffer) {
        return -1;
    }

    ethernet_frame_header_t *header = buffer;
    memcpy(header->source_mac, iface->hwaddr, NET_HWADDR_LEN_ETHERNET);
    memcpy(header->destination_mac, dst, NET_HWADDR_LEN_ETHERNET);
    header->ethertype = htons(protocol);
    memcpy((uint8_t *)buffer + sizeof(*header), payload, length);

    int result = network_send(iface->device, buffer, frame_length);
    kfree(buffer);
    return result;
}

const net_link_ops_t net_ethernet_link_ops = {
    .name = "ethernet",
    .address_length = NET_HWADDR_LEN_ETHERNET,
    .header_length = sizeof(ethernet_frame_header_t),
    .send = net_ethernet_send,
    .receive = net_ethernet_receive,
};
