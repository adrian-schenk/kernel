#include "sdt.h"
#include "printf.h"
#include "string.h"
#include <stdint.h>

int apic_ids[128] = {0};
unsigned char apic_ids_count = 0;

struct MADT {
    struct SDTHeader header;
    uint32_t local_apic_address;
    uint32_t flags;
} __attribute__((packed));

struct MADTEntryHeader {
    uint8_t type;
    uint8_t length;
} __attribute__((packed));

struct MADTLocalAPIC {
    uint8_t type;
    uint8_t length;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed));

struct MADTLocalX2APIC {
    uint8_t type;
    uint8_t length;
    uint16_t reserved;
    uint32_t x2apic_id;
    uint32_t flags;
    uint32_t processor_uid;
} __attribute__((packed));



struct SDTHeader* RSDT = 0;
struct SDTHeader* XSDT = 0;
struct SDTHeader* MADT = 0;

uint64_t find_rsdp() {
    for(int i=0;i<0x100000;i+=16) {
        if(memcmp8(i,"RSD PTR "))return (uint64_t)i;
    }

    return 0;
}

void rsdp_setup(struct XSDP_t* ptr) {
    if(ptr == 0) {
        return;
    }

    if (rsdp_checksum((struct RSDP_t*) ptr) != 0) {
        return;
    }

    if(ptr->Revision == 0) {
        // RSDT
        if (ptr->RsdtAddress == 0) {
            return;
        }

        struct SDTHeader* rsdt = (struct SDTHeader*) ptr->RsdtAddress;
        RSDT = rsdt;
        
        struct SDTHeader* madt = sdt_find(rsdt, "APIC");
        MADT = madt;
    } else {
        // XSDT
        if (ptr->Length < sizeof(struct XSDP_t) || xsdt_checksum(ptr) != 0) {
            return;
        }

        if (ptr->XsdtAddress == 0) {
            return;
        }

        struct SDTHeader* xsdt = (struct SDTHeader*) ptr->XsdtAddress;
        XSDT = xsdt;

        struct SDTHeader* madt = xsdt_find(xsdt, "APIC");
        MADT = madt;
    }

}

struct SDTHeader *sdt_find(struct SDTHeader* ptr, char* signature) {
    if (ptr == 0 || ptr->Length < sizeof(struct SDTHeader)) {
        return 0;
    }
    
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

struct SDTHeader *xsdt_find(struct SDTHeader* xsdt, char* signature) {
    if (xsdt == 0 || xsdt->Length < sizeof(struct SDTHeader)) {
        return 0;
    }

    int entries = (xsdt->Length - sizeof(struct SDTHeader)) / 8;
    uint64_t* entry_ptr = (uint64_t*) (xsdt + 1);
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

void enumerate_madt_cores() {
    apic_ids_count = 0;
    if (MADT == 0) {
        return;
    }

    struct MADT* madt = (struct MADT*)MADT;
    if (madt->header.Length < sizeof(struct MADT)) {
        return;
    }


    uint8_t* entry = ((uint8_t*)madt) + sizeof(struct MADT);
    uint8_t* end = ((uint8_t*)madt) + madt->header.Length;
    int cpu_count = 0;

    while (entry + sizeof(struct MADTEntryHeader) <= end) {
        struct MADTEntryHeader* hdr = (struct MADTEntryHeader*)entry;

        if (hdr->length < sizeof(struct MADTEntryHeader) || entry + hdr->length > end) {
            break;
        }

        if (hdr->type == 0 && hdr->length >= sizeof(struct MADTLocalAPIC)) {
            struct MADTLocalAPIC* lapic = (struct MADTLocalAPIC*)entry;
            int enabled = (lapic->flags & 0x1) != 0;
            int online_capable = (lapic->flags & 0x2) != 0;
            kprintf("CPU[%d] ID:%d apic-id:%d\n",
                cpu_count,
                lapic->processor_id,
                lapic->apic_id);
            cpu_count++;
            apic_ids[apic_ids_count++] = lapic->apic_id;
        } else if (hdr->type == 9 && hdr->length >= sizeof(struct MADTLocalX2APIC)) {
            struct MADTLocalX2APIC* x2apic = (struct MADTLocalX2APIC*)entry;
            int enabled = (x2apic->flags & 0x1) != 0;
            int online_capable = (x2apic->flags & 0x2) != 0;
            kprintf("CPU[%d] ID:%d apic-id:%d\n",
                cpu_count,
                x2apic->processor_uid,
                x2apic->x2apic_id
            );
            cpu_count++;
            apic_ids[apic_ids_count++] = x2apic->x2apic_id;
        }

        entry += hdr->length;
    }

    kprintf("MADT CPU entries found: %d\n", cpu_count);
}