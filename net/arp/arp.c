#include <arp/arp.h>
#include <printf.h>
#include <e1000.h>
#include <ethernet/ethernet.h>
#include <endian.h>

void handle_arp_request(arp_request_t *request) {
    do_arp_reply(request);
}

void do_arp_reply(arp_request_t *request) {
    arp_reply_t reply = {
        .hardware_type = request->hardware_type,
        .protocol_type = request->protocol_type,
        .hardware_size = request->hardware_size,
        .protocol_size = request->protocol_size,
        .opcode = htons(2), // ARP Reply
    };
    
    reply.sender_ip[0] = 10;
    reply.sender_ip[1] = 0;
    reply.sender_ip[2] = 2;
    reply.sender_ip[3] = 2; // Example sender IP

    reply.sender_mac[0] = 0x00;
    reply.sender_mac[1] = 0x1A;
    reply.sender_mac[2] = 0x2B;
    reply.sender_mac[3] = 0x3C;
    reply.sender_mac[4] = 0x4D;
    reply.sender_mac[5] = 0x5E; // Example sender MAC

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

    ethernet_frame_header_t frame;
    frame.destination_mac[0] = reply.target_mac[0];
    frame.destination_mac[1] = reply.target_mac[1];
    frame.destination_mac[2] = reply.target_mac[2];
    frame.destination_mac[3] = reply.target_mac[3];
    frame.destination_mac[4] = reply.target_mac[4];
    frame.destination_mac[5] = reply.target_mac[5];

    frame.source_mac[0] = reply.sender_mac[0];
    frame.source_mac[1] = reply.sender_mac[1];
    frame.source_mac[2] = reply.sender_mac[2];
    frame.source_mac[3] = reply.sender_mac[3];
    frame.source_mac[4] = reply.sender_mac[4];
    frame.source_mac[5] = reply.sender_mac[5];
    frame.ethertype = htons(0x0806); // ARP Ethertype in little-endian

    e1000_network_send_tx((void *)&frame, sizeof(ethernet_frame_header_t) + sizeof(arp_reply_t));
}