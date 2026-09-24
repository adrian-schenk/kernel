#include <ethernet/ethernet.h>
#include <arp/arp.h>
#include <ip/ip.h>

void handle_ethernet_frame(ethernet_frame_header_t *frame, uint16_t length) {
    char src_mac[18], dst_mac[18];
    uint16_t ethertype = (frame->ethertype << 8) | (frame->ethertype >> 8); // Convert to host byte order

    switch (ethertype) {
        case 0x0800: // IPv4
            mac_to_string(frame->source_mac, src_mac);
            mac_to_string(frame->destination_mac, dst_mac);
            mac_to_string(frame->destination_mac, dst_mac);
            kprintf("Received IPv4 packet: Src MAC: %s, Dst MAC: %s, Length: %u\n", src_mac, dst_mac, length);
            ip4_handle_packet((const void *)((uint8_t *)frame + sizeof(ethernet_frame_header_t)), length - sizeof(ethernet_frame_header_t));
            break;
        case 0x0806: // ARP
            mac_to_string(frame->source_mac, src_mac);
            mac_to_string(frame->destination_mac, dst_mac);
            kprintf("Received ARP packet: Src MAC: %s, Dst MAC: %s, Length: %u\n", src_mac, dst_mac, length);
            handle_arp_request((arp_request_t *)((uint8_t *)frame + sizeof(ethernet_frame_header_t)));
            break;
        default:
            mac_to_string(frame->source_mac, src_mac);
            mac_to_string(frame->destination_mac, dst_mac);
            kprintf("Received unknown Ethernet frame: Ethertype: 0x%04X, Src MAC: %s, Dst MAC: %s, Length: %u\n", ethertype, src_mac, dst_mac, length);
            break;
    }
}