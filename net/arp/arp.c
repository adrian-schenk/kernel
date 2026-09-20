#include <arp/arp.h>
#include <printf.h>
#include <network.h>
#include <ethernet/ethernet.h>
#include <endian.h>

void handle_arp_request(arp_request_t *request) {
    do_arp_reply(request);
}

void do_arp_reply(arp_request_t *request) {
    const uint8_t *local_mac = network_primary_mac();
    const uint8_t *local_ip = network_primary_ipv4();

    if (!local_mac || !local_ip) {
        return;
    }

    arp_reply_t reply = {
        .hardware_type = request->hardware_type,
        .protocol_type = request->protocol_type,
        .hardware_size = request->hardware_size,
        .protocol_size = request->protocol_size,
        .opcode = htons(2), // ARP Reply
    };
    
    for (int index = 0; index < 4; index++) {
        reply.sender_ip[index] = local_ip[index];
    }

    for (int index = 0; index < 6; index++) {
        reply.sender_mac[index] = local_mac[index];
    }

    reply.target_ip[0] = request->sender_ip[0];
    reply.target_ip[1] = request->sender_ip[1];
    reply.target_ip[2] = request->sender_ip[2];
    reply.target_ip[3] = request->sender_ip[3];

    reply.target_mac[0] = request->sender_mac[0];
    reply.target_mac[1] = request->sender_mac[1];
    reply.target_mac[2] = request->sender_mac[2];
    reply.target_mac[3] = request->sender_mac[3];
    reply.target_mac[4] = request->sender_mac[4];
    reply.target_mac[5] = request->sender_mac[5];

    struct {
        ethernet_frame_header_t frame;
        arp_reply_t reply;
    } packet;

    for (int index = 0; index < 6; index++) {
        packet.frame.destination_mac[index] = reply.target_mac[index];
        packet.frame.source_mac[index] = reply.sender_mac[index];
    }
    packet.frame.ethertype = htons(0x0806); // ARP Ethertype in little-endian
    packet.reply = reply;

    network_send(&packet, sizeof(packet));
}