#include "sdt.h"
#include "string.h"
#include <stdint.h>

struct SDTHeader* RSDT = 0;
struct SDTHeader* XSDT = 0;
struct SDTHeader* MADT = 0;
struct SDTHeader* FADT = 0;
struct SDTHeader* SRAT = 0;
struct SDTHeader* SSDT = 0;

uint64_t find_rsdp() {
    for(int i=0;i<0x100000;i+=16) {
        if(memcmp8(i,"RSD PTR "))return (uint64_t)i;
    }
}

void rsdp_setup(struct XSDP_t* ptr) {
    if(ptr->Revision == 0) {
        // RSDT
        if (rsdp_checksum((struct RSDP_t*) ptr) != 0) {
            // invalid checksum
            return;
        }

        struct SDTHeader* rsdt = (struct SDTHeader*) ptr->RsdtAddress;
        RSDT = rsdt;
        
        struct SDTHeader* madt = sdt_find((struct RSDP_t*) rsdt, "APIC");
        MADT = madt;
    } else {
        // XSDT
        struct SDTHeader* xsdt = (struct SDTHeader*) ptr->XsdtAddress;
        XSDT = xsdt;

        struct SDTHeader* madt = xsdt_find(ptr, "APIC");
        MADT = madt;
    }

}

struct SDTHeader *sdt_find(struct SDTHeader* ptr, char* signature) {
    int entries = (ptr->Length - sizeof(struct SDTHeader)) / 4;
    uint32_t* entry_ptr = (uint32_t*) (ptr + 1);
    for(int i=0;i<entries;i++) {
        struct SDTHeader* header = (struct SDTHeader*) entry_ptr[i];
        if(memcmp4(header->Signature, signature)) {
            return header;
        }
    }
    return 0;

}

struct SDTHeader *xsdt_find(struct XSDP_t* ptr, char* signature) {
    int entries = (ptr->Length - sizeof(struct XSDP_t)) / 8;
    uint64_t* entry_ptr = (uint64_t*) (ptr + 1);
    for(int i=0;i<entries;i++) {
        struct SDTHeader* header = (struct SDTHeader*) entry_ptr[i];
        if(memcmp8(header->Signature, signature)) {
            return header;
        }
    }
    return 0;
}

int rsdp_checksum(struct RSDP_t* ptr) {
    uint8_t sum = 0;
    for(int i=0;i<20;i++) {
        sum += ((uint8_t*)ptr)[i];
    }
    return sum;
}

int xsdt_checksum(struct XSDP_t* ptr) {
    uint8_t sum = 0;
    for(int i=0;i<ptr->Length;i++) {
        sum += ((uint8_t*)ptr)[i];
    }
    return sum % 0x100 == 0;
}