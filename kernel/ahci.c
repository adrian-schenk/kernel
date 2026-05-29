#include "ahci.h"
#include "printf.h"
#include "page.h"
#include "endian.h"
#include "pci.h"

void probe_ports(hba_mem_t *abar);
static int check_type(hba_port_t *port);

void ahci_init() {
		// Find AHCI controller
		pci_device_t *hba = pci_find_class_subclass(0x01, 0x06);
		if (!hba)
			return;
		hba_mem_t *abar = (void*)(uintptr_t)hba;
		for (int i = 0; i < 6; i++) {
			if (!hba->bars[i].is_io && hba->bars[i].base != 0) {
				abar = (void*)(uintptr_t)hba->bars[i].base;
				break;
			}
		}

		probe_ports(abar);

		hba_port_t *port = &abar->ports[0];
		
		end_cmd(port);

		identify_device(port);

		void *region = pt_alloc_page_phys(1);

		hba_cmd_hdr_t *cmd_hdrs = region;
		port->clb = (uint32_t)(uintptr_t)region;
		port->fb = pt_alloc_page_phys(1);
		port->is = (uint32_t)-1;

		region = pt_alloc_page_phys(1);
		hba_cmd_tbl_t *cmd_tbl = region;

		char cmdslot = ahci_get_cmdslot(port);
		if (cmdslot == -1)
			return;

		cmd_hdrs[cmdslot].prdtl = 1; // 1 entry in prdt
		cmd_hdrs[cmdslot].ctba = (uint32_t)(uintptr_t)cmd_tbl;
		cmd_hdrs[cmdslot].cfl = sizeof(fis_reg_h2d_t) / 4;

		// read prdt 4KB for now
		cmd_tbl->prdt_entry[0].dba = pt_alloc_page_phys(1);
		cmd_tbl->prdt_entry[0].dbc = 4096 - 1;

		fis_reg_h2d_t *cfis =
    (fis_reg_h2d_t*)cmd_tbl->cfis;

		uint32_t startl = 0, starth = 0, count = 1; // read first 4 sectors (512 bytes each)
		cfis->fis_type = FIS_TYPE_REG_H2D;
		cfis->c = 1; // command
		cfis->command = 0x25; // read dma ext
		cfis->lba0 = (uint8_t)startl;
		cfis->lba1 = (uint8_t)(startl>>8);
		cfis->lba2 = (uint8_t)(startl>>16);
		cfis->device = 1<<6;	// LBA mode

		cfis->lba3 = (uint8_t)(startl>>24);
		cfis->lba4 = (uint8_t)starth;
		cfis->lba5 = (uint8_t)(starth>>8);

		cfis->countl = count & 0xFF;
		cfis->counth = (count >> 8) & 0xFF;

		while (abar->ports[0].tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ));

		start_cmd(&abar->ports[0]);
		port->ci = 1; // issue command

		while (port->ci & 1); // wait until completion

		if (port->serr == 0)
		{
			printf("Read successful!\n");
		}
		else
		{
			printf("Read failed! serr: %x\n", port->serr);
		}

		printf("%s", (char*)(uintptr_t)cmd_tbl->prdt_entry[0].dba);
}

int ahci_read(hba_port_t *port, uint64_t start_lba, uint16_t sector_count, void* buf) {	
	return 0;
}

int ahci_write(hba_port_t *port, uint64_t start_lba, uint16_t sector_count, void* buf) {
	return 0;
}

char ahci_get_cmdslot(hba_port_t *port) {
	// If not set in shadow register, return error
	if ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) != 0)
		return -1;

	for (char i = 0; i < 32; i++) {
		if ((port->sact & (1 << i)) == 0)
			return i;
	}
	return -1;
}

void start_cmd(hba_port_t *port) {
	while (port->cmd & HBA_PxCMD_CR); // wait until CR is cleared

	port->cmd |= HBA_PxCMD_FRE; // enable FIS receive
	port->cmd |= HBA_PxCMD_ST;  // start command engine

}

void end_cmd(hba_port_t *port) {
	port->cmd &= ~HBA_PxCMD_ST;  // stop command engine
	port->cmd &= ~HBA_PxCMD_FRE; // disable FIS receive

	while (port->cmd & HBA_PxCMD_CR); // wait until CR is cleared

}

void probe_ports(hba_mem_t *abar)
{
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
	int i = 0;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar->ports[i]);
			if (dt == AHCI_DEV_SATA)
			{
				printf("SATA drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				printf("SATAPI drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				printf("SEMB drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_PM)
			{
				printf("PM drive found at port %d\n", i);
			}
		}

		pi >>= 1;
		i ++;
	}
}

void identify_device(hba_port_t *port)
{
	while (port->tfd & ATA_DEV_BUSY); // wait until not busy

	void *region = pt_alloc_page_phys(1);

	hba_cmd_hdr_t *cmd_hdrs = region;
	port->clb = (uint32_t)(uintptr_t)region;
	port->fb = pt_alloc_page_phys(1);
	port->is = (uint32_t)-1;

	region = pt_alloc_page_phys(1);
	hba_cmd_tbl_t *cmd_tbl = region;

	char cmdslot = ahci_get_cmdslot(port);
	if (cmdslot == -1)
		return;

	cmd_hdrs[cmdslot].prdtl = 1; // 1 entry in prdt
	cmd_hdrs[cmdslot].ctba = (uint32_t)(uintptr_t)cmd_tbl;
	cmd_hdrs[cmdslot].cfl = sizeof(fis_reg_h2d_t) / 4;

	// read prdt 512 bytes for now
	uint16_t *buf = cmd_tbl->prdt_entry[0].dba = pt_alloc_page_phys(1);
	cmd_tbl->prdt_entry[0].dbc = 512 - 1;

	fis_reg_h2d_t *cfis =
	(fis_reg_h2d_t*)cmd_tbl->cfis;

	cfis->fis_type = FIS_TYPE_REG_H2D;
	cfis->c = 1; // command
	cfis->command = 0xEC; // identify device
	cfis->device = 0;
	
	while (port->fb & (ATA_DEV_BUSY | ATA_DEV_DRQ));

	start_cmd(port);
	port->ci = 1; // issue command

	while (port->ci & 1); // wait until completion

	if (port->serr == 0)
	{
		uint64_t sectors = (uint64_t)buf[100];
		char model_str[41];
		for (int i = 0; i < 20; i += 2)
		{
			*((uint16_t*)(model_str + i)) = ntohs(*(uint16_t*)((char*)((uint32_t*)buf + 13) + i));
		}
		model_str[40] = '\0';
		uint64_t size_bytes = sectors * 512;
		printf("Found Drive: %s with size %d!\n", model_str, size_bytes);
	}
	else
	{
		printf("Identify failed! serr: %x\n", port->serr);
	}

	end_cmd(port);
}

// Check device type
static int check_type(hba_port_t *port)
{
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	switch (port->sig)
	{
    case SATA_SIG_ATAPI:
      return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
      return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
      return AHCI_DEV_PM;
    default:
      return AHCI_DEV_SATA;
	}
}