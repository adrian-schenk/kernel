#pragma once
#include <stddef.h>
#include <stdint.h>
#include "printf.h"

#define PCI_MAX_BUSES 1
#define PCI_MAX_DEVICES 32
#define PCI_MAX_FUNCTIONS 8

typedef struct pci_bar {
  uint32_t base;
  char prefetchable;
  char is_io;
} pci_bar_t;

typedef struct pci_device {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    pci_bar_t bars[6];
} pci_device_t;

extern pci_device_t *pci_devices;
extern int pci_device_count;

void pci_init();

uint16_t pci_read_16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t pci_read_8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_read_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_check_vendor(uint8_t bus, uint8_t slot);

void pci_check_all_buses();

static void pci_scan_function(uint8_t bus, uint8_t device, uint8_t function);
static void pci_scan_device(uint8_t bus, uint8_t device);
static void pci_find_ahci();

pci_device_t *pci_find_class_subclass(int class, int subclass);