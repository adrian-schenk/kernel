#include "e1000.h"
#include "pci.h"
#include "mmio.h"
#include "page.h"
#include "printf.h"

static const char hex_digits[] = "0123456789ABCDEF";

static void mac_to_string(const uint8_t mac[6], char out[18])
{
    int j = 0;
    for (int i = 0; i < 6; i++)
    {
        out[j++] = hex_digits[mac[i] >> 4];
        out[j++] = hex_digits[mac[i] & 0xF];
        if (i < 5)
            out[j++] = ':';
    }
    out[j] = '\0';
}

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

    uint64_t base = (uint64_t)bar;

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
}
