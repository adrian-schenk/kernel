#include "e1000.h"
#include "pci.h"
#include "mmio.h"
#include "page.h"
#include "printf.h"
#include "sleep.h"
#include <ethernet/ethernet.h>

void *base;
void *rings;
int rx_descriptors;
int tx_descriptors;

int cur_rx = 0;
int cur_tx = 0;

void e1000_init(void)
{
    pci_device_t *dev = pci_find_class_subclass(0x02, 0x00);
    if (!dev)
    {
        kprintf("e1000: no network controller found\n");
        return;
    }

    if (dev->vendor_id != 0x8086 || dev->device_id != 0x100E)
    {
        kprintf("e1000: unsupported network controller %x:%x\n",
                dev->vendor_id, dev->device_id);
        return;
    }

    /* Find the memory-mapped BAR (BAR0 on the 82540EM). */
    uint32_t bar = 0;
    for (int i = 0; i < 6; i++)
    {
        if (!dev->bars[i].is_io && dev->bars[i].base != 0)
        {
            bar = dev->bars[i].base;
            break;
        }
    }
    if (!bar)
    {
        kprintf("e1000: no memory-mapped BAR found\n");
        return;
    }

    /* Enable bus mastering and memory space. */
    uint16_t cmd = pci_read_16(dev->bus, dev->device, dev->function, 0x04);
    cmd |= (1u << 2) | (1u << 1) | (1u << 0);
    pci_write_16(dev->bus, dev->device, dev->function, 0x04, cmd);

    /* Identity-map the MMIO register space. */
    for (uint64_t offset = 0; offset < E1000_MMIO_SIZE; offset += PAGE_SIZE)
    {
        pt_map_page(&kernel_pml4,
                    (uint64_t)bar + offset,
                    (uint64_t)bar + offset,
                    PAGE_PRESENT | PAGE_WRITABLE);
    }

    base = (void *)(uint64_t)bar;

    /* Software reset; the card clears the bit once done. */
    mmio_writel(base + E1000_CTRL, mmio_readl(base + E1000_CTRL) | E1000_CTRL_RST);
    while (mmio_readl(base + E1000_CTRL) & E1000_CTRL_RST)
        ;

    /* After reset the MAC is auto-loaded from the EEPROM into the Receive
     * Address registers: RAL holds bytes 0-3, RAH holds bytes 4-5. */
    uint32_t ral = mmio_readl(base + E1000_RAL);
    uint32_t rah = mmio_readl(base + E1000_RAH);

    uint8_t mac[6] = {
        (uint8_t)(ral & 0xFF),
        (uint8_t)((ral >> 8) & 0xFF),
        (uint8_t)((ral >> 16) & 0xFF),
        (uint8_t)((ral >> 24) & 0xFF),
        (uint8_t)(rah & 0xFF),
        (uint8_t)((rah >> 8) & 0xFF),
    };

    char mac_str[18];
    mac_to_string(mac, mac_str);
    kprintf("e1000: MAC %s\n", mac_str);


    void *phys_page = pt_alloc_page_phys(10);

    rings = phys_page;
    void *buffers = (void *)((uint64_t)phys_page + PAGE_SIZE);

    rx_descriptors = 1;
    e1000_receive_descriptor_t *rx_ring = (e1000_receive_descriptor_t *)rings;
    rx_ring->buffer_addr = (uint64_t)buffers;

    // Initialize the receive ring.
    mmio_writel(base + E1000_RDBAL, (uint32_t)(uint64_t)rx_ring);
    mmio_writel(base + E1000_RDBAH, (uint32_t)((uint64_t)rx_ring >> 32));
    mmio_writel(base + E1000_RDLEN, sizeof(e1000_receive_descriptor_t) * 1);
    mmio_writel(base + E1000_RDH, 0);
    mmio_writel(base + E1000_RDT, rx_descriptors);

    mmio_writel(base + E1000_RCTL, 1 << 1 | 1 << 15 | 3 << 16 | 1 << 25); // Enable receiver, strip CRC, broadcast accept, BSIZE = 4096
    
    tx_descriptors = 8;
    e1000_transmit_descriptor_t *tx_ring = (e1000_transmit_descriptor_t *)((uint64_t)rings + PAGE_SIZE / 2);
    for (int i = 0; i < tx_descriptors; i++) {
        tx_ring[i].buffer_addr = (uint64_t)buffers + 2048 * (i + 1);
    }
    
    // Initialize the transmit ring.
    mmio_writel(base + E1000_TDBAL, (uint32_t)(uint64_t)tx_ring);
    mmio_writel(base + E1000_TDBAH, (uint32_t)((uint64_t)tx_ring >> 32));
    mmio_writel(base + E1000_TDLEN, sizeof(e1000_transmit_descriptor_t) * tx_descriptors);
    mmio_writel(base + E1000_TDH, 0);
    mmio_writel(base + E1000_TDT, 0);
    
    mmio_writel(base + E1000_TCTL, 1 << 1 | 1 << 3 | 1 << 4 | 1 << 5); // Enable transmitter, pad short packets, collision threshold

    kprintf("e1000: RX ring at %p, TX ring at %p, buffers at %p\n", rx_ring, tx_ring, buffers);
}

void e1000_network_send_tx(void* data, unsigned int len) {
    e1000_transmit_descriptor_t *tx_ring = (e1000_transmit_descriptor_t *)((uint64_t)rings + PAGE_SIZE / 2);
    tx_ring = &tx_ring[cur_tx];

    tx_ring->cmd = 0b00001011; // Set the command bits: EOP, IFCS, RS
    tx_ring->length = len;
    tx_ring->buffer_addr = (uint64_t)data;

    mmio_writel(base + E1000_TDT, ++cur_tx); // Update the Transmit Descriptor Tail to indicate a new packet is ready
    cur_tx %= tx_descriptors;
    
}

void e1000_network_rx_handler() {

    for (unsigned int i = 0; i < (unsigned int)rx_descriptors; ) {
        
        e1000_receive_descriptor_t *rx_ring = (e1000_receive_descriptor_t *)rings;
        e1000_receive_descriptor_t *desc = &rx_ring[i];

        if (desc->status & 0x01) { // Check if the descriptor is done

            ethernet_frame_header_t *eth_hdr = (ethernet_frame_header_t *)(uintptr_t)desc->buffer_addr;
            handle_ethernet_frame(eth_hdr, desc->length);

            // Process the received packet here (desc->buffer_addr points to the data)
            desc->status = 0; // Clear the status to indicate it's free
        }

        i = (i + 1) % rx_descriptors; // Wrap around the ring buffer
    }
}