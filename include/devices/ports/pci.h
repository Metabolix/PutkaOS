#ifndef _PCI_H
#define _PCI_H 1

int pci_init (void);

typedef struct _pciDev_t
{
	uint16_t vendor_id;
	uint16_t device_id;
	
	uint16_t command;
	uint16_t status;
	
	uint8_t revision_id;
	uint8_t interface;
	uint8_t sub_class;
	uint8_t base_class;
	
	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;

	uint8_t bus;
	uint8_t dev;
	uint8_t func;

	uint8_t irq;
	uint32_t base[6];
	uint32_t size[6];
	uint8_t type[6];
	uint32_t rom_base;
	uint32_t rom_size;

	uint16_t subsys_vendor;
	uint16_t subsys_device;

	uint8_t current_state;
} pciDev_t;

pciDev_t *pciGetDeviceById(uint16_t vendor, uint32_t device, int index);

#endif
