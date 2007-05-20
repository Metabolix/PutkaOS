#include <storage/ide.h>
#include <storage/hdd.h>
#include <io.h>
#include <timer.h>
#include <time.h>
#include <string.h>
#include <endian.h>
#include <debugprint.h>

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
	ATA_BYTES_PER_SECTOR,
	-1,
	0,
	(read_one_block_t)  ata_read_one_sector,
	(write_one_block_t) ata_write_one_sector,
	(read_blocks_t)  ata_read,
	(write_blocks_t) ata_write
};

// params[0] = device
// params[1] = waitcode
// params[2] = on_off
int ide_wait_condition(uint_t * params)
{
	uint_t result;
	if (params[0] < IDE_NUM_DEVICES) {
		result = inportb(ide_ports[params[0] >> 1].comStat) & params[1];
		if ( (params[2] && result) || (!params[2] && !result) ) {
			return 0;
		}
	}
	return -1;
}

int ide_wait(uint_t device, uint_t waitcode, uint_t on_off)
{
	uint_t params[3];
	params[0] = device;
	params[1] = waitcode;
	params[2] = on_off;
	if (kwait_until_0(0, IDE_TIMEOUT, (waitfunc_t) ide_wait_condition, (waitparam_t) (params)))
		kprintf("WARNING, WAIT TIMED OUT!\n");
	return 0;//? IDE_ERROR_TIMED_OUT : 0;
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
	if (ide_wait(device, 0x08, 1)) {
		return IDE_ERROR_TIMED_OUT;
	}
	ata_read_next_sector(device, (uint16_t*)buf);
	return 0;
}

int atapi_identify (uint_t device, void * buf)
{
	ide_select_device(device);
	outportb(ide_ports[device >> 1].comStat, 0xA1);
	if (ide_wait(device, 0x08, 1)) {
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

		ide_devices[device].blockdev.read_blocks = (read_blocks_t) atapi_read;
		ide_devices[device].blockdev.read_one_block = (read_one_block_t) atapi_read_one_sector;
		ide_devices[device].blockdev.write_blocks = 0;
		ide_devices[device].blockdev.write_one_block = 0;

		// luetaan levyn tiedot
		atapi_start(device);
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
		if ((ide_wait(device, 0x08, 1) != 0)
		|| (rw_next_sector(device, (uint16_t*)(buf + i * ATA_BYTES_PER_SECTOR)) != 0)) {
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

		sector += 0xff * ATA_BYTES_PER_SECTOR;
		buf += 0xff * ATA_BYTES_PER_SECTOR;
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

int atapi_start(int device)
{
	uint16_t in_word;
	uint_t num_sectors, sector_size;
	// käynnistetään moottori?
	atapi_send_packet(device, 0, (uint16_t*)ATAPI_PACKET_START);
	// luetaan tiedot
	atapi_send_packet(device, 0, (uint16_t*)ATAPI_PACKET_READCAPACITY);


	//ide_wait(device, IDE_DRV_DRQ, 1);
	in_word = inportw(ide_ports[device >> 1].data);
	num_sectors = ((in_word & 0x0FF) << 24) | ((in_word & 0xFF00) << 8);


	//ide_wait(device, IDE_DRV_DRQ, 1);
	in_word = inportw(ide_ports[device >> 1].data);
	num_sectors |= ((in_word & 0x0FF) << 8) | ((in_word & 0xFF00) >> 8);


	//ide_wait(device, IDE_DRV_DRQ, 1);
	in_word = inportw(ide_ports[device >> 1].data);
	sector_size = ((in_word & 0x0FF) << 24) | ((in_word & 0xFF00) << 8);


	//ide_wait(device, IDE_DRV_DRQ, 1);
	in_word = inportw(ide_ports[device >> 1].data);
	sector_size |= ((in_word & 0x0FF) << 8) | ((in_word & 0xFF00) >> 8);

	kprintf("Sector count: %d, Sector size: %d\n", num_sectors, sector_size);

	//ide_devices[device].blockdev.std.remove = atapi_safely_remove;
	//ide_devices[device].blockdev.block_size = sector_size;

	if (!num_sectors || !sector_size) {
		ide_devices[device].blockdev.block_size = 0;
		ide_devices[device].blockdev.block_count = 0;
		ide_devices[device].media_available = 0;
		return -1;
	}
	ide_devices[device].media_available = 1;
	ide_devices[device].blockdev.block_size = ATAPI_BYTES_PER_SECTOR;
	ide_devices[device].blockdev.block_count = num_sectors;
	return 0;
}

int atapi_reset(int device)
{
	ide_wait(device, IDE_CTRL_BSY, 1);
	outportb(ide_ports[device >> 1].featErr, 0xCC);
	outportb(ide_ports[device >> 1].comStat, 0xEF);
	kwait(0, 1000);
	ide_wait(device, IDE_CTRL_BSY, 1);
	//itse reset
	outportb(ide_ports[device >> 1].comStat, 0x08);
	kwait(0, 1000);
	ide_wait(device, IDE_CTRL_BSY, 1);

	return 0;
}

int atapi_send_packet(int device, uint_t bytecount, uint16_t * packet)
{
	int i;

	while (inportb(ide_ports[device >> 1].comStat) & 0x80);
	while (!(inportb(ide_ports[device >> 1].comStat) & 0x40));

	outportb(ide_ports[device >> 1].driveHead, (device & 0x01) << 4); //laite 0 = 0x00, laite 1 olis 0x10
	outportb(ide_ports[device >> 1].altComStat, 0x0A); //skipataan keskeytys
	outportb(ide_ports[device >> 1].comStat, 0xA0); //THE COMMAND

	kwait(0, 1000);

	while (inportb(ide_ports[device >> 1].comStat) & 0x80);
	while (!(inportb(ide_ports[device >> 1].comStat) & 0x08));

	for (i = 0; i < 6; i++) {
		outportw(ide_ports[device >> 1].data, packet[i]);
		inportb(ide_ports[device >> 1].altComStat);
	}

	inportb(ide_ports[device >> 1].altComStat);

	while (inportb(ide_ports[device >> 1].comStat) & 0x80);

	return 0;
}

size_t atapi_real_read(int device, uint32_t sector, size_t count, uint16_t * buffer)
{
	uint16_t word_count = 0;
	uint8_t status = 0;
	int i = 0;
	size_t read_words = 0;
	size_t words_to_read;

	words_to_read = count * (ATAPI_BYTES_PER_SECTOR >> 1);

	if (!ide_devices[device].media_available) {
		if (atapi_start(device) != 0) {
			return -1;
		}
	}

	while (inportb(ide_ports[device >> 1].altComStat) & 0x08) {
		if (inportb(ide_ports[device >> 1].altComStat) & 0x01) {
			//virhe
			return -1;
		}
	}

	kwait(0, 1000);

	unsigned char ATAPI_command[] = {
		0xA8, 0,
		(unsigned char)(((sector) >> 24) & 0xff),
		(unsigned char)(((sector) >> 16) & 0xff),
		(unsigned char)(((sector) >>  8) & 0xff),
		(unsigned char)(((sector) >>  0) & 0xff),
		(unsigned char)(((count) >> 24) & 0xff),
		(unsigned char)(((count) >> 16) & 0xff),
		(unsigned char)(((count) >>  8) & 0xff),
		(unsigned char)(((count) >>  0) & 0xff),
		0, 0
	};

	atapi_send_packet(device, 0xFFFF, (uint16_t*)(ATAPI_command));

	for (;;) {
		status = inportb(ide_ports[device >> 1].comStat) & 0x08;
		status |= (inportb(ide_ports[device >> 1].sectorCount) & 0x03);

		switch (status) {
			case 0x03:
				//done
				if (read_words < words_to_read) {
					DEBUGP("Didn't get as much data as wanted...\n");
					return ATAPI_CONV_WORDS_TO_SECTORS(read_words);
				}
				return count;
			case 0x00:
				//abort
				kprintf("atapi_read: Command aborted.\n");
				return ATAPI_CONV_WORDS_TO_SECTORS(read_words);
			default: {}
				// break;
		}

		while (!(inportb(ide_ports[device >> 1].altComStat) & 0x08)) {
			if (inportb(ide_ports[device >> 1].altComStat) & 0x01) {
				return ATAPI_CONV_WORDS_TO_SECTORS(read_words);
			}
		}

		word_count = inportb(ide_ports[device >> 1].cylinderLow);
		word_count |= ((uint16_t)(inportb(ide_ports[device >> 1].cylinderHigh))) << 8;
		word_count >>= 1;

		for (i = 0; i < word_count; i++) {
			buffer[read_words + i] = inportw(ide_ports[device >> 1].data);
		}

		read_words += word_count;
	}

	// Not reached
	return 0;
}

size_t atapi_read(ide_device_t *device, uint64_t sector, size_t count, void * buf)
{
	return atapi_real_read(device->devnum, sector, count, buf);
}
int atapi_read_one_sector (ide_device_t *device, uint64_t sector, void * buf)
{
	return (atapi_real_read(device->devnum, sector, 1, buf) == 1) ? 0 : -1;
}
