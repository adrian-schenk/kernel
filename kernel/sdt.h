#pragma once
#include <stdint.h>

extern struct SDTHeader* RSDT;
extern struct SDTHeader* XSDT;
extern struct SDTHeader* MADT;
extern struct SDTHeader* FADT;
extern struct SDTHeader* SRAT;
extern struct SDTHeader* SSDT;

struct RSDP_t {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
} __attribute__ ((packed));

struct XSDP_t {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;      // deprecated since version 2.0

 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed));

struct SDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
};

void rsdp_setup(struct XSDP_t* ptr);
uint64_t find_rsdp();
int rsdp_checksum(struct RSDP_t* ptr);
struct SDTHeader *sdt_find(struct SDTHeader* ptr, char* signature);

struct SDTHeader *xsdt_find(struct XSDP_t* ptr, char* signature);
int xsdt_checksum(struct XSDP_t* ptr);