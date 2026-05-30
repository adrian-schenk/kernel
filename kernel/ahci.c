#include "ahci.h"
#include "printf.h"
#include "page.h"
#include "endian.h"
#include "pci.h"
#include "buddy_alloc.h"

hba_mem_t *abar;

port_t ports[32];

drive_t drives[32];
int drive_count = 0;

int probe_port(hba_mem_t *abar, int port);
static int check_type(hba_port_t *port);

void ahci_init()
{
	// Find AHCI controller
	pci_device_t *hba = pci_find_class_subclass(0x01, 0x06);
	if (!hba)
		return;
	abar = (void *)(uintptr_t)hba;
	for (int i = 0; i < 6; i++)
	{
		if (!hba->bars[i].is_io && hba->bars[i].base != 0)
		{
			abar = (void *)(uintptr_t)hba->bars[i].base;
			break;
		}
	}

	for (int i = 0; i < 32; i++)
	{
		if (probe_port(abar, i) == 1)
		{
			ports[i].port = &abar->ports[i];
			ports[i].cmd_hdrs = pt_alloc_page_phys(1);
			ports[i].free_slots = 0xFFFFFFFF;

			hba_port_t *port = &abar->ports[i];
			stop_cmd(port);

			port->fb = pt_alloc_page_phys(1);
			port->clb = (uint32_t)(uintptr_t)ports[i].cmd_hdrs;
			identify_device(port);

		}
	}
}

int ahci_read(hba_port_t *port, uint64_t addr, uint64_t size, void *buf)
{
	if (!drives[port - abar->ports].size || size == 0)
		return -1;

	if ((addr % 512) || (size % 512))
		return -1;

	uint64_t drive_size = drives[port - abar->ports].size;

	if (addr + size > drive_size)
		return -1;

	stop_cmd(port);

	uint64_t lba = addr / 512;
	uint32_t count = size / 512;

	char slot = ahci_get_cmdslot(port - abar->ports);
	if (slot == -1)
		return -1;

	hba_cmd_hdr_t *cmd_hdrs = (hba_cmd_hdr_t *)(uintptr_t)port->clb;
	hba_cmd_tbl_t *cmd_tbl = &ports[port - abar->ports].cmd_tbls[slot];

	// memset(cmd_tbl, 0, 4096);

	cmd_hdrs[slot].w = 0;
	cmd_hdrs[slot].cfl = sizeof(fis_reg_h2d_t) / 4;
	cmd_hdrs[slot].prdtl = 1;
	cmd_hdrs[slot].ctba = (uint32_t)(uintptr_t)cmd_tbl;

	uint64_t remaining = size;
	uint64_t offset = 0;
	int i = 0;

	while (remaining && i < MAX_PRDT_ENTRIES)
	{
		uint32_t bytes = remaining > PRDT_MAX_BYTES ? PRDT_MAX_BYTES : remaining;

		uint64_t phys = (uint64_t)(uintptr_t)buf + offset; // MUST be phys in real kernel

		cmd_tbl->prdt_entry[i].dba = (uint32_t)phys;
		cmd_tbl->prdt_entry[i].dbc = bytes - 1;
		cmd_tbl->prdt_entry[i].i = 1;

		remaining -= bytes;
		offset += bytes;
		i++;
	}

	fis_reg_h2d_t *cfis = (fis_reg_h2d_t *)cmd_tbl->cfis;
	// memset(cfis, 0, sizeof(fis_reg_h2d_t));

	cfis->fis_type = FIS_TYPE_REG_H2D;
	cfis->c = 1;
	cfis->command = 0x25; // READ DMA EXT

	cfis->lba0 = (uint8_t)lba;
	cfis->lba1 = (uint8_t)(lba >> 8);
	cfis->lba2 = (uint8_t)(lba >> 16);
	cfis->lba3 = (uint8_t)(lba >> 24);
	cfis->lba4 = (uint8_t)(lba >> 32);
	cfis->lba5 = (uint8_t)(lba >> 40);

	cfis->device = (1 << 6);

	cfis->countl = count & 0xFF;
	cfis->counth = (count >> 8) & 0xFF;

	while (port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ))
		;

	start_cmd(port);

	port->ci = (1U << slot);

	ahci_free_cmdslot(port - abar->ports, slot);
	while (port->ci & (1U << slot))
	{
		if (port->is & HBA_PxIS_TFES)
			return -1;
	}

	return 0;
}

int ahci_write(hba_port_t *port, uint64_t addr, uint64_t size, void *buf)
{
	stop_cmd(port);

	if (!drives[port - abar->ports].size || size == 0)
		return -1;

	if ((addr % 512) || (size % 512))
		return -1;

	uint64_t drive_size = drives[port - abar->ports].size;

	if (addr + size > drive_size)
		return -1;

	uint64_t lba = addr / 512;
	uint32_t count = size / 512;

	char slot = ahci_get_cmdslot(port - abar->ports);
	if (slot == -1)
		return -1;

	hba_cmd_hdr_t *cmd_hdrs = (hba_cmd_hdr_t *)(uintptr_t)port->clb;
	hba_cmd_tbl_t *cmd_tbl = &ports[port - abar->ports].cmd_tbls[slot];

	// memset(cmd_tbl, 0, 4096);

	cmd_hdrs[slot].w = 1;
	cmd_hdrs[slot].cfl = sizeof(fis_reg_h2d_t) / 4;
	cmd_hdrs[slot].prdtl = 1;
	cmd_hdrs[slot].ctba = (uint32_t)(uintptr_t)cmd_tbl;

	uint64_t remaining = size;
	uint64_t offset = 0;
	int i = 0;

	while (remaining && i < MAX_PRDT_ENTRIES)
	{
		uint32_t bytes = remaining > PRDT_MAX_BYTES ? PRDT_MAX_BYTES : remaining;

		uint64_t phys = (uint64_t)(uintptr_t)buf + offset; // MUST be phys in real kernel

		cmd_tbl->prdt_entry[i].dba = (uint32_t)phys;
		cmd_tbl->prdt_entry[i].dbc = bytes - 1;
		cmd_tbl->prdt_entry[i].i = 1;

		remaining -= bytes;
		offset += bytes;
		i++;
	}

	fis_reg_h2d_t *cfis = (fis_reg_h2d_t *)cmd_tbl->cfis;
	// memset(cfis, 0, sizeof(fis_reg_h2d_t));

	cfis->fis_type = FIS_TYPE_REG_H2D;
	cfis->c = 1;
	cfis->command = 0x35; // WRITE DMA EXT

	cfis->lba0 = (uint8_t)lba;
	cfis->lba1 = (uint8_t)(lba >> 8);
	cfis->lba2 = (uint8_t)(lba >> 16);
	cfis->lba3 = (uint8_t)(lba >> 24);
	cfis->lba4 = (uint8_t)(lba >> 32);
	cfis->lba5 = (uint8_t)(lba >> 40);

	cfis->device = (1 << 6);

	cfis->countl = count & 0xFF;
	cfis->counth = (count >> 8) & 0xFF;

	while (port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ))
		;

	start_cmd(port);

	port->ci = (1U << slot);

	ahci_free_cmdslot(port - abar->ports, slot);
	while (port->ci & (1U << slot))
	{
		if (port->is & HBA_PxIS_TFES)
			return -1;
	}

	return 0;
}

char ahci_get_cmdslot(int portnum)
{
	port_t *port = &ports[portnum];
	// If not set in shadow register, return error
	if ((port->port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) != 0)
		return -1;

	for (char i = 0; i < 32; i++)
	{
		if ((port->free_slots & (1 << i)) != 0)
		{
			port->free_slots &= ~(1 << i);
			return i;
		}
	}
	return -1;
}

void ahci_free_cmdslot(int portnum, char slot)
{
	port_t *port = &ports[portnum];
	port->free_slots |= (1 << slot);
}

void start_cmd(hba_port_t *port)
{
	while (port->cmd & HBA_PxCMD_CR)
		; // wait until CR is cleared

	port->cmd |= HBA_PxCMD_FRE; // enable FIS receive
	port->cmd |= HBA_PxCMD_ST;	// start command engine
}

void stop_cmd(hba_port_t *port)
{
	port->cmd &= ~HBA_PxCMD_ST;	 // stop command engine
	port->cmd &= ~HBA_PxCMD_FRE; // disable FIS receive

	while (port->cmd & HBA_PxCMD_CR)
		; // wait until CR is cleared
}

int probe_port(hba_mem_t *abar, int port)
{
	// Search disk in implemented ports
	uint32_t pi = abar->pi;

	int i = port;
	if (pi & (1 << i))
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
		else {
			return 0;
		}

		return 1;
	}

	return 0;
}

void identify_device(hba_port_t *port)
{
	
	while (port->tfd & ATA_DEV_BUSY)
		; // wait until not busy

	hba_cmd_hdr_t *cmd_hdrs = port->clb;
	port->is = (uint32_t)-1;

	char cmdslot = ahci_get_cmdslot(port - abar->ports);
	if (cmdslot == -1)
	return;
	
	hba_cmd_tbl_t *cmd_tbl = &ports[port - abar->ports].cmd_tbls[cmdslot];
	cmd_hdrs[cmdslot].prdtl = 1; // 1 entry in prdt
	cmd_hdrs[cmdslot].ctba = (uint32_t)(uintptr_t)cmd_tbl;
	cmd_hdrs[cmdslot].cfl = sizeof(fis_reg_h2d_t) / 4;

	// read prdt 512 bytes for now
	uint16_t *buf = cmd_tbl->prdt_entry[0].dba = pt_alloc_page_phys(1);
	cmd_tbl->prdt_entry[0].dbc = 512 - 1;

	fis_reg_h2d_t *cfis =
			(fis_reg_h2d_t *)cmd_tbl->cfis;

	cfis->fis_type = FIS_TYPE_REG_H2D;
	cfis->c = 1;					// command
	cfis->command = 0xEC; // identify device
	cfis->device = 0;

	while (port->fb & (ATA_DEV_BUSY | ATA_DEV_DRQ))
		;

	start_cmd(port);
	port->ci = 1; // issue command

	while (port->ci & 1)
		; // wait until completion

	if (port->serr == 0)
	{
		uint64_t sectors = (uint64_t)buf[100];
		char model_str[41];
		for (int i = 0; i < 20; i += 2)
		{
			*((uint16_t *)(model_str + i)) = ntohs(*(uint16_t *)((char *)((uint32_t *)buf + 13) + i));
		}
		model_str[40] = '\0';
		uint64_t size_bytes = sectors * 512;
		printf("Found Drive: %s with size %d!\n", model_str, size_bytes);

		drives[drive_count].port = port - abar->ports;
		drives[drive_count].size = size_bytes;
		strncpy(drives[drive_count].model, model_str, 40);
		drive_count++;
	}
	else
	{
		printf("Identify failed! serr: %x\n", port->serr);
	}

	stop_cmd(port);
}

// Check device type
static int check_type(hba_port_t *port)
{
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT) // Check drive status
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