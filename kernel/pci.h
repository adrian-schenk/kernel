#pragma once
#include <stddef.h>
#include <stdint.h>
#include "printf.h"

#define PCI_MAX_BUSES 256
#define PCI_MAX_DEVICES 32
#define PCI_MAX_FUNCTIONS 8

void pci_init();

uint16_t pci_read_16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t pci_read_8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_read_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_check_vendor(uint8_t bus, uint8_t slot);

void pci_check_all_buses();

static void pci_scan_function(uint8_t bus, uint8_t device, uint8_t function);
static void pci_scan_device(uint8_t bus, uint8_t device);
static void pci_find_ahci();