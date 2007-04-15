#include <storage/ide.h>
#include <storage/hdd.h>
#include <io.h>
#include <timer.h>
#include <time.h>
#include <string.h>
#include <endian.h>

#include <screen.h>

// tuetaan ide-kontrolleri 0:aa ja 1:stä
static ide_controller_t ide_ports[IDE_NUM_CONTROLLERS] = {
	{ 0x01F0, 0x01F1, 0x01F2, 0x01F3, 0x01F4, 0x01F5, 0x01F6, 0x01F7, 0x03F6 },
	{ 0x0170, 0x0171, 0x0172, 0x0173, 0x0174, 0x0175, 0x0176, 0x0177, 0x0376 }
};

ide_device_t ide_devices[IDE_NUM_DEVICES];

uint_t used_controllers;

const BD_DEVICE ide_blockdev = {
	{
		0,
		DEV_CLASS_BLOCK,
		DEV_TYPE_ERROR,
		-1,
		(devopen_t) blockdev_fopen,
		(devrm_t) ata_safely_remove
	},
	IDE_BYTES_PER_SECTOR,
	-1,
	0,
	(read_one_block_t)  ata_read_one_sector,
	(write_one_block_t) ata_write_one_sector,
	(read_blocks_t)  ata_read,
	(write_blocks_t) ata_write
};

int ide_wait_condition(uint_t device)
{
	if (device < IDE_NUM_DEVICES) {
		if (inportb(ide_ports[device >> 1].comStat) & 0x08) {
			return 0;
		}
	}
	return -1;
}

int ide_wait(uint_t device)
{
	return kwait_until_0(0, IDE_TIMEOUT, (waitfunc_t) ide_wait_condition, (waitparam_t) device) ? IDE_ERROR_TIMED_OUT : 0;
}

int ide_controller_exists(uint_t controller)
{
	outportb(ide_ports[controller].sectorNumber, 0x88);
	kwait(0, 1000);

	if (inportb(ide_ports[controller].sectorNumber) != 0x88) {
		return IDE_ERROR_DOESNT_EXIST;
	}

	return 0;
}

int ide_init(void)
{
	// Lasketaan kontrollerit
	int controller;

	used_controllers = 0;

	for (controller = 0; controller < IDE_NUM_CONTROLLERS; controller++) {
		if (ide_controller_exists(controller) == 0) {
			used_controllers |= (0x01 << controller);
		}
	}

	ide_probe();
	return 0;
}

void ide_probe(void)
{
	int controller, dev_loop;

	for (controller = 0; controller < IDE_NUM_CONTROLLERS; controller++) {
		for (dev_loop = 0; dev_loop < IDE_DEVICES_PER_CONTROLLER; dev_loop++) {
			if (used_controllers & (0x01 << controller)) {
				int d = dev_loop + (controller * 2);
				ide_identify_device(controller, d);
			}
		}
	}
}

int ide_select_device(uint_t device)
{
	unsigned char data;

	if (device >= IDE_NUM_DEVICES) {
		return IDE_ERROR_INVALID_PARAMETER;
	}

	//	      device 0 vai 1
	data = ((device & 0x01) << 4) | 0xA0;

	outportb(ide_ports[device >> 1].driveHead, data);

	return 0;
}

int ide_reset(uint_t controller)
{
	if (controller >= used_controllers) {
		return IDE_ERROR_INVALID_PARAMETER;
	}

	outportb(ide_ports[controller].altComStat, 0x04);
	kwait(0, 1000);

	outportb(ide_ports[controller].altComStat, 0x00);
	kwait(0, 1000);

	//odotetaan kunnes laite on toimintavalmiina
	extern void taikatemppu(void);
	while (inportb(ide_ports[controller].altComStat) & IDE_CTRL_BSY) taikatemppu();

	if (inportb(ide_ports[controller].altComStat) & IDE_DRV_ERR) {
		kprintf("Virhe ide_reset:ssa!\n");
	}

	return 0;
}

int ata_identify (uint_t device, void * buf)
{
	outportb(ide_ports[device >> 1].data,            0x00);
	outportb(ide_ports[device >> 1].sectorCount,     0x00);
	outportb(ide_ports[device >> 1].sectorNumber,    0x00);
	outportb(ide_ports[device >> 1].cylinderLow,     0x00);
	outportb(ide_ports[device >> 1].cylinderHigh,    0x00);
	ide_select_device(device);
	outportb(ide_ports[device >> 1].comStat,         0xEC);
	if (ide_wait(device)) {
		return IDE_ERROR_TIMED_OUT;
	}
	ata_read_next_sector(device, (uint16_t*)buf);
	return 0;
}

int atapi_identify (uint_t device, void * buf)
{
	ide_select_device(device);
	outportb(ide_ports[device >> 1].comStat, 0xA1);
	if (ide_wait(device)) {
		return IDE_ERROR_TIMED_OUT;
	}
	ata_read_next_sector(device, (uint16_t*)buf);
	return 0;
}

void ide_identify_device(uint_t controller, uint_t device)
{
	uint16_t buffer[256];

	memset(&ide_devices[device].blockdev + 1, 0, sizeof(ide_device_t) - sizeof(BD_DEVICE));
	ide_devices[device].blockdev = ide_blockdev;
	ide_devices[device].devnum = device;
	ide_reset(controller);
	ide_select_device(device);

	if (inportb(ide_ports[controller].sectorCount) != 1 || inportb(ide_ports[controller].sectorNumber) != 1) {
		return;
	}

	inportb(ide_ports[controller].comStat); // tarpeellinen?
	unsigned short cyl =
		+ (inportb(ide_ports[controller].cylinderLow))
		+ (inportb(ide_ports[controller].cylinderHigh) << 8);

	switch (cyl) {
		case 0x9669:
			ide_devices[device].ide_type |= IDE_DEV_SERIAL;
		case 0xeb14:
			ide_devices[device].ide_type |= IDE_DEV_ATAPI;
			ide_devices[device].ide_type |= IDE_DEV_KNOWN;
			break;

		case 0xc33c:
			ide_devices[device].ide_type |= IDE_DEV_SERIAL;
		case 0:
			ide_devices[device].ide_type |= IDE_DEV_KNOWN;
			break;
		default:
			ide_devices[device].blockdev.std.dev_type = DEV_TYPE_ERROR;
			return; // Ei tunneta, pois.
	}

	if (ide_devices[device].ide_type & IDE_DEV_ATAPI) {
		// on ATAPI
		atapi_identify(device, buffer);
		// tyyppi CD-ROM
		if ((buffer[0] & 0x1F00) == 0x0500) {
			ide_devices[device].blockdev.std.dev_type = DEV_TYPE_CDROM;
		} else {
			ide_devices[device].blockdev.std.dev_type = DEV_TYPE_OTHER;
		}
	} else {
		// on ATA, joten käytetään ATA-IDENTIFY:ä
		ata_identify(device, buffer);
		ide_devices[device].blockdev.block_count = *((uint32_t*)(buffer + 60));
		ide_devices[device].blockdev.std.dev_type = DEV_TYPE_HARDDISK;
	}

	word_bytes_swap_memcpy(ide_devices[device].serial_number, buffer + 10, 20);
	word_bytes_swap_memcpy(ide_devices[device].firmware, buffer + 23, 8);
	word_bytes_swap_memcpy(ide_devices[device].model, buffer + 27, 40);

	if (buffer[0] & 0x80) {
		ide_devices[device].removable = 1;
	}

	// TODO: Nimi laitteelle
	char *name = kmalloc(5);
	name[0] = 'c';
	name[1] = controller + '0';
	name[2] = 'd';
	name[3] = (device&1) + '0';
	name[4] = 0;
	ide_devices[device].blockdev.std.name = name;

	// Lahjoitetaan se devmanagerille
	device_insert((DEVICE*) &ide_devices[device]);

	// Osiot yms...
	switch (ide_devices[device].blockdev.std.dev_type) {
		case DEV_TYPE_HARDDISK:
			hdd_read_partitions(&ide_devices[device]);
			break;
		default:
			break;
	}
}

int ata_read_next_sector (uint_t device, uint16_t * buf)
{
	int i;
	for (i = 0; i < 256; i++) {
		buf[i] = inportw(ide_ports[device >> 1].data);
	}

	return 0;
}

int ata_write_next_sector (uint_t device, uint16_t * buf)
{
	int i;

	for (i = 0; i < 256; i++) {
		outportw(ide_ports[device >> 1].data, buf[i]);
	}

	return 0;
}

int setup_LBA (uint_t device, uint64_t sector, uint8_t count)
{
	outportb(ide_ports[device >> 1].sectorCount, count);
	outportb(ide_ports[device >> 1].sectorNumber, sector);
	outportb(ide_ports[device >> 1].cylinderLow, sector >> 8);
	outportb(ide_ports[device >> 1].cylinderHigh, sector >> 16);
	// osoitteen bitit 25 -> 28 + laitteen tunnus, käytetään LBA:ta ym.
	outportb(ide_ports[device >> 1].driveHead, 0xE0 | ((device & 0x01) << 4) | (((uint32_t)sector >> 24) & 0x0F));

	return 0;
}

size_t ata_rw_some (uint_t device, uint64_t sector, uint8_t count, char * buf, ata_rw_next_sector_t rw_next_sector)
{
	int i;
	size_t retval;

	setup_LBA(device, sector, count);
	// lukukomento
	outportb(ide_ports[device >> 1].comStat, 0x20);

	retval = 0;
	for (i = 0; i < count; i++) {
		if ((ide_wait(device) != 0)
		|| (rw_next_sector(device, (uint16_t*)(buf + i * IDE_BYTES_PER_SECTOR)) != 0)) {
			return retval;
		}
		++retval;
	}

	return retval;
}

size_t ata_rw (uint_t device, uint64_t sector, size_t count, char * buf, ata_rw_next_sector_t rw_next_sector)
{
	size_t retval, some_retval;

	retval = 0;
	while (count > 0xff) {
		some_retval = ata_rw_some(device, sector, 0xff, buf, rw_next_sector);

		retval += some_retval;
		if (some_retval != 0xff) {
			return retval;
		}

		sector += 0xff * IDE_BYTES_PER_SECTOR;
		buf += 0xff * IDE_BYTES_PER_SECTOR;
		count -= 0xff;
	}
	retval += ata_rw_some(device, sector, count, (char*) buf, rw_next_sector);
	return retval;
}

size_t ata_read (ide_device_t *device, uint64_t sector, size_t count, void * buf)
{
	return ata_rw(device->devnum, sector, count, (char*) buf, ata_read_next_sector);
}

size_t ata_write (ide_device_t *device, uint64_t sector, size_t count, void * buf)
{
	return ata_rw(device->devnum, sector, count, (char*) buf, ata_write_next_sector);
}

int ata_read_one_sector (ide_device_t *device, uint64_t sector, void * buf)
{
	return ata_rw(device->devnum, sector, 1, (char*) buf, ata_read_next_sector) ? 0 : -1;
}

int ata_write_one_sector (ide_device_t *device, uint64_t sector, void * buf)
{
	return ata_rw(device->devnum, sector, 1, (char*) buf, ata_write_next_sector) ? 0 : -1;
}

int ata_safely_remove(ide_device_t *device)
{
	if (!device || !device->removable) {
		return -1; // Fail
	}
	return 0; // Success
}
