#include "pci.h"
#include "printf.h"
#include "io.h"

void pci_init() {
  pci_check_all_buses();
  pci_find_ahci();
}

void pci_check_all_buses() {
    for (uint8_t bus = 0; bus < 1; bus++) {
        for (uint8_t device = 0; device < PCI_MAX_DEVICES; device++) {
            pci_scan_device(bus, device);
        }
    }
}

uint16_t pci_read_16(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address =
        (1u << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)func << 8) |
        (offset & 0xFC);

    outl(0xCF8, address);
    return (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pci_read_8(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    return (pci_read_16(bus, device, func, offset & ~1) >> ((offset & 1) * 8)) & 0xFF;
}

uint32_t pci_read_32(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address =
        (1u << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)func << 8) |
        (offset & 0xFC);

    outl(0xCF8, address);
    return inl(0xCFC);
}

uint16_t pci_check_vendor(uint8_t bus, uint8_t slot) {
    return pci_read_16(bus, slot, 0, 0);
}

static const char* pci_class_name(uint8_t class_code, uint8_t subclass) {
    if (class_code == 0x06 && subclass == 0x00) return "Host bridge";
    if (class_code == 0x06 && subclass == 0x01) return "ISA bridge";
    if (class_code == 0x03 && subclass == 0x00) return "VGA controller";
    if (class_code == 0x02) return "Network controller";
    if (class_code == 0x01) return "Mass storage controller";
    return "Unknown device";
}

static void pci_print_bar(uint32_t bar, int index) {
    if (bar == 0) return;

    if (bar & 0x1) {
        printf("      BAR%d: I/O at 0x%x\n", index, bar & ~0x3);
    } else {
        printf("      BAR%d: %s memory at 0x%x\n",
               index,
               (bar & 0x8) ? "prefetchable" : "non-prefetchable",
               bar & ~0xF);
    }
}

static void pci_scan_function(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor = pci_read_16(bus, device, function, 0x00);
    if (vendor == 0xFFFF) return;

    uint16_t device_id = pci_read_16(bus, device, function, 0x02);
    uint8_t class_code = pci_read_8(bus, device, function, 0x0B);
    uint8_t subclass   = pci_read_8(bus, device, function, 0x0A);
    uint8_t header_type = pci_read_8(bus, device, function, 0x0E);

    printf("  Bus %d, device %d, function %d:\n", bus, device, function);

    printf("    %s: PCI device %x:%x (%x, %x)\n",
           pci_class_name(class_code, subclass),
           vendor,
           device_id,
           class_code,
           subclass);

    // BARs (0–5)
    for (int i = 0; i < 6; i++) {
        uint32_t bar = pci_read_32(bus, device, function, 0x10 + i * 4);
        pci_print_bar(bar, i);
    }

    printf("      id \"\"\n");

    // multifunction device check
    if (!(header_type & 0x80)) return;
}

static void pci_scan_device(uint8_t bus, uint8_t device) {
    uint16_t vendor = pci_read_16(bus, device, 0, 0x00);
    if (vendor == 0xFFFF) return;
    
    uint8_t header_type = pci_read_8(bus, device, 0, 0x0E);

    for (uint8_t function = 0; function < 8; function++) {
        if (function == 0 || (header_type & 0x80)) {
            pci_scan_function(bus, device, function);
        }
    }
}

static void pci_find_ahci() {
    for (uint8_t bus = 0; bus < 1; bus++) {
        for (uint8_t device = 0; device < PCI_MAX_DEVICES; device++) {
            for (uint8_t function = 0; function < PCI_MAX_FUNCTIONS; function++) {
                uint16_t vendor = pci_read_16(bus, device, function, 0x00);
                if (vendor == 0xFFFF) continue;

                uint8_t class_code = pci_read_8(bus, device, function, 0x0B);
                uint8_t subclass   = pci_read_8(bus, device, function, 0x0A);

                if (class_code == 0x01 && subclass == 0x06) {
                    printf("AHCI controller found at bus %d, device %d, function %d\n", bus, device, function);
                    
                }
            }
        }
    }
}