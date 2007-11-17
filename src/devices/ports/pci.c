#include <io.h>
#include <memory/malloc.h>
#include <screen.h>
#include <devices/ports/pci.h>
#include <devices/ports/pci_list.h>

#define PCI_HEADER_TYPE_NORMAL		0
#define PCI_HEADER_TYPE_BRIDGE		1
#define PCI_HEADER_TYPE_CARDBUS		2

#define PCI_COMMAND					0x04
#define PCI_STATUS					0x06
#define PCI_IRQ_LINE				0x3C
#define PCI_IRQ_PIN					0x3D
#define PCI_SUBSYSTEM_VENDOR_ID		0x2C
#define PCI_SUBSYSTEM_ID			0x2E
#define PCI_LATENCY_TIMER			0x0D
#define PCI_CAPABILITY_LIST		    0x34
#define PCI_CB_CAPABILITY_LIST		0x14
#define PCI_CACHE_LINE_SIZE		    0x0C
#define PCI_CB_SUBSYSTEM_VENDOR_ID	0x40
#define PCI_CB_SUBSYSTEM_ID		    0x42
#define PCI_ROM_ADDRESS				0x30
#define PCI_ROM_ADDRESS_1			0x38

#define PCI_BASE_ADDRESS_0				0x10
#define PCI_BASE_ADDRESS_SPACE			0x01
#define PCI_BASE_ADDRESS_SPACE_MEMORY	0x00
#define PCI_IO_RESOURCE_MEM				0x00
#define PCI_IO_RESOURCE_IO				0x01

#define PCI_ROM_ADDRESS_MASK		(~0x7FFUL)
#define PCI_BASE_ADDRESS_MEM_MASK	(~0x0FUL )
#define PCI_BASE_ADDRESS_IO_MASK	(~0x03UL )
#define PCI_PM_CTRL_STATE_MASK		( 0x0003 )

#define PCI_ROM_ADDRESS_ENABLE	0x01
#define PCI_COMMAND_IO			0x01
#define PCI_COMMAND_MEMORY		0x02
#define PCI_COMMAND_MASTER		0x04

//Taulukko PCI-laitteista
static pciDev_t **pciDevices = NULL;
static int pciCount = 0;

struct cfg_cmd
{
	unsigned reg:8;
	unsigned func:3;
	unsigned dev:5;
	unsigned bus:8;
	unsigned reserved:7;
	unsigned enable:1;
};

union cmd
{
	struct cfg_cmd cc;
	uint32_t ci;
};

char *pciGetVendorStr(uint16_t vendor)
{
	int i;
	static char ven[] = "UNKNOWN";
	for(i=0;i<PCI_VENTABLE_LEN;i++) {
		 if(PciVenTable[i].VenId == vendor) {
			return PciVenTable[i].VenShort;
		}
	}
	return ven;
}


char *pciGetDevStr(uint16_t ven, uint16_t dev)
{
	int i;
	static char devstr[] = "UNKNOWN";
	for(i=0; i<PCI_DEVTABLE_LEN;i++) {
		if(PciDevTable[i].VenId == ven && PciDevTable[i].DevId == dev) {
			return PciDevTable[i].ChipDesc;
		}
	}

	return devstr;
}

uint32_t pciReadConfigDword(int bus, int dev, int func, int reg)
{
	uint16_t base;
	union cmd cmd;

	cmd.cc.enable = 1;
	cmd.cc.reserved = 0;
	cmd.cc.bus = bus & 0xff;
	cmd.cc.dev = dev & 0x1f;
	cmd.cc.func = func & 0x07;
	cmd.cc.reg = reg & 0xfc;

	outportdw(0xcf8, cmd.ci);
	base = 0xcfc + (reg & 0x03);

	return inportdw(base);
}


uint16_t pciReadConfigWord(int bus, int dev, int func, int reg)
{
	uint16_t base;
	union cmd cmd;

	cmd.cc.enable = 1;
	cmd.cc.reserved = 0;
	cmd.cc.bus = bus & 0xFF;
	cmd.cc.dev = dev & 0x1F;
	cmd.cc.func = func & 0x7;
	cmd.cc.reg = reg & 0xFC;

	outportdw(0xCF8, cmd.ci);
	base = 0xCFC + (reg & 0x03);

	return inportw(base);
}

uint8_t pciReadConfigByte(int bus, int dev, int func, int reg)
{
    uint16_t base;
    union cmd cmd;

	cmd.cc.enable = 1;
    cmd.cc.reserved = 0;
    cmd.cc.bus = bus & 0xFF;
    cmd.cc.dev = dev & 0x1F;
    cmd.cc.func = func & 0x7;
    cmd.cc.reg = reg & 0xFC;

    outportdw(0xCF8, cmd.ci);
    base = 0xCFC + (reg & 0x03);

    return inportb(base);
}

void pciWriteConfigDword(int bus, int dev, int func, int reg, uint32_t data)
{
    uint16_t base;
    union cmd cmd;

	cmd.ci = 0;
	cmd.cc.enable = 1;
    cmd.cc.reserved = 0;
    cmd.cc.bus = bus;
    cmd.cc.dev = dev;
    cmd.cc.func = func;
    cmd.cc.reg = reg & 0xFC;

	base = 0xcfc + (reg & 0x03);
    outportdw(0xCF8, cmd.ci);
	outportdw(base, data);
}

void pciWriteConfigWord(int bus, int dev, int func, int reg, uint16_t data)
{
    uint16_t base;
    union cmd cmd;

	cmd.ci = 0;
	cmd.cc.enable = 1;
    cmd.cc.reserved = 0;
    cmd.cc.bus = bus;
    cmd.cc.dev = dev;
    cmd.cc.func = func;
    cmd.cc.reg = reg & 0xFC;

    base = 0xcfc + (reg & 0x03);
    outportdw(0xCF8, cmd.ci);
    outportw(base, data);
}

void pciWriteConfigByte(int bus, int dev, int func, int reg, uint8_t data)
{
    uint16_t base;
    union cmd cmd;

	cmd.ci = 0;
	cmd.cc.enable = 1;
    cmd.cc.reserved = 0;
    cmd.cc.bus = bus;
    cmd.cc.dev = dev;
    cmd.cc.func = func;
    cmd.cc.reg = reg & 0xFC;

    base = 0xcfc + (reg & 0x03);
    outportdw(0xCF8, cmd.ci);
    outportb(base, data);
}

static void pciReadIrq(pciDev_t *p)
{
	uint8_t irq;
	irq = pciReadConfigByte(p->bus, p->dev, p->func, PCI_IRQ_PIN);
	if(irq) {
		irq = pciReadConfigByte(p->bus, p->dev, p->func, PCI_IRQ_LINE);
	}
	p->irq = irq;
}

uint32_t pciSize(uint32_t base, uint32_t mask)
{
	uint32_t size = mask & base;
	size = size & ~(size-1);
	return(size-1);
}

static void pciReadBases(pciDev_t *p, int count, int rom)
{
	int i;
	uint32_t l, sz, reg;

	memset(p->base, 0, sizeof(p->base));
	memset(p->size, 0, sizeof(p->size));
	memset(p->type, 0, sizeof(p->type));

	for(i=0; i<count; i++) {
		reg = PCI_BASE_ADDRESS_0 + (i << 2);
		l = pciReadConfigDword(p->bus, p->dev, p->func, reg);
		pciWriteConfigDword(p->bus, p->dev, p->func, reg, ~0);

		sz = pciReadConfigDword(p->bus, p->dev, p->func, reg);
		pciWriteConfigDword(p->bus, p->dev, p->func, reg, l);

		if(!sz || sz == 0xffffffff) {
			continue;
		}

		if(l == 0xffffffff) {
			l = 0;
		}

		if((l & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY) {
			p->base[i] = l & PCI_BASE_ADDRESS_MEM_MASK;
			p->size[i] = pciSize(sz, PCI_BASE_ADDRESS_MEM_MASK);
			p->type[i] = PCI_IO_RESOURCE_MEM;
		} else {
			p->base[i] = l & PCI_BASE_ADDRESS_IO_MASK;
			p->size[i] = pciSize(sz, PCI_BASE_ADDRESS_IO_MASK);
			p->type[i] = PCI_IO_RESOURCE_IO;
		}
	}

	if(rom) {
		p->rom_base = 0;
		p->rom_size = 0;

		l = pciReadConfigDword(p->bus, p->dev, p->func, rom);
		pciWriteConfigDword(p->bus, p->dev, p->func,
							rom, ~PCI_ROM_ADDRESS_ENABLE);
		sz = pciReadConfigDword(p->bus, p->dev, p->func, rom);
		pciWriteConfigDword(p->bus, p->dev, p->func, rom, l);

		if (sz && sz != 0xFFFFFFFF) {
			p->rom_base = l & PCI_ROM_ADDRESS_MASK;
			sz = pciSize(sz, PCI_ROM_ADDRESS_MASK);
			p->rom_size = p->rom_size + (unsigned long)sz;
		}
	}
}


uint16_t pci_probe(int bus, int dev, int func, pciDev_t *pciDev)
{
	int i;
	uint32_t *temp = (uint32_t *)pciDev;

	for(i=0; i<4; i++) {
		temp[i] = pciReadConfigDword(bus, dev, func, i<<2);
	}

	if(pciDev->vendor_id == 0xffff || pciDev->vendor_id == 0x0) {
		//Ei ole kiinni mitään tässä paikassa.
		return 0;
	}
	pciDev->bus = bus;
	pciDev->func = func;
	pciDev->dev = dev;

	pciDev->current_state = 4;

	switch(pciDev->header_type) {
		case PCI_HEADER_TYPE_NORMAL:
			pciReadIrq(pciDev);
			pciReadBases(pciDev, 6, PCI_ROM_ADDRESS);
			pciDev->subsys_vendor =
					pciReadConfigWord(bus, dev, func, PCI_SUBSYSTEM_VENDOR_ID);
			pciDev->subsys_device =
					pciReadConfigWord(bus, dev, func, PCI_SUBSYSTEM_ID);
			kprintf("%s - %s %x:%x\n",
					pciGetVendorStr(pciDev->vendor_id),
					pciGetDevStr(pciDev->vendor_id, pciDev->device_id),
					pciDev->vendor_id, pciDev->device_id);
			break;
		case PCI_HEADER_TYPE_BRIDGE:
			pciReadBases(pciDev, 2, PCI_ROM_ADDRESS_1);
			kprintf("%s - %s %x:%x\n",
					pciGetVendorStr(pciDev->vendor_id),
					pciGetDevStr(pciDev->vendor_id, pciDev->device_id),
					pciDev->vendor_id, pciDev->device_id);
			break;
		case PCI_HEADER_TYPE_CARDBUS:
			pciReadIrq(pciDev);
			pciReadBases(pciDev, 1, 0);
            pciDev->subsys_vendor = pciReadConfigWord(bus, dev, func, PCI_SUBSYSTEM_VENDOR_ID);
            pciDev->subsys_device = pciReadConfigWord(bus, dev, func, PCI_SUBSYSTEM_ID);
			kprintf("%s - %s %x:%x\n",
					pciGetVendorStr(pciDev->vendor_id),
					pciGetDevStr(pciDev->vendor_id, pciDev->device_id),
					pciDev->vendor_id, pciDev->device_id);
			break;
		default:
			break;
	}

	return 1;
}

pciDev_t *pciGetDeviceById(uint16_t vendor, uint32_t device, int index)
{
	int i = 0;
	int c = 0;

	while(i < pciCount) {
		if(pciDevices[i]->vendor_id == vendor && pciDevices[i]->device_id == device) {
			c++;
			if(c == index) {
				return pciDevices[i];
			}
		}
		i++;
	}
	return NULL;
}

int pci_init (void)
{
	pciDev_t pciDev;
	//onko PCI:tä?
	outportdw (0xCF8, 0x80000000);
	if(inportdw(0xCF8) != 0x80000000) {
		kprintf("PCI not found\n");
		return -1;
	}
	kprintf("PCI found, probing PCI bus...\n");

	for (int bus = 0; bus < 4; bus++) {
		for (int dev = 0; dev < 32; dev++) {
			for (int func = 0; func < 8; func++) {
				if (pci_probe(bus, dev, func, &pciDev)) {
					if (func && !(pciDev.header_type & 0x80)) {
						// Löytyi monta tästä, jatketaan.
						continue;
					}
					pciCount++;
					// Ensimmäinen?
					if (pciDevices) {
						pciDevices = (pciDev_t **)krealloc(pciDevices, sizeof(pciDev_t *) * pciCount);
					} else {
						pciDevices = (pciDev_t **)kmalloc(sizeof(pciDev_t *)* pciCount);
					}
					pciDevices[pciCount-1] = (pciDev_t *)kmalloc(sizeof(pciDev_t));
					memcpy(pciDevices[pciCount-1], &pciDev,
							sizeof(pciDev_t));
				}
			}
		}
	}
    kprintf("PCI probe done!\n");
	return 0;
}

