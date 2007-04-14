#ifndef _IDE_H
#define _IDE_H 1

#include <stdint.h>
#include <devmanager.h>
#include <blockdev.h>

#define ATA_ERROR_INVALID_PARAMETER -1
#define ATA_ERROR_TIMED_OUT -2

#define DEVICES_PER_CONTROLLER 2

#define NUM_CONTROLLERS 2
#define NUM_DEVICES (DEVICES_PER_CONTROLLER * NUM_CONTROLLERS)

#define IDE_CTRL_BSY   0x80
#define IDE_DRV_RDY    0x40
#define IDE_DRV_WRTFLT 0x20
#define IDE_DRV_SKCOMP 0x10
#define IDE_DRV_DRQ    0x08
#define IDE_DRV_CORDAT 0x04
#define IDE_DRV_IDX    0x02
#define IDE_DRV_ERR    0x01

#define IDE_CTRL_1 0x00
#define IDE_CTRL_2 0x01

#define IDE_DEV_1 0x00
#define IDE_DEV_2 0x01

#define ATA_DEV_KNOWN     	0x01
#define ATA_DEV_SERIAL    	0x02
#define ATA_DEV_ATAPI     	0x04
#define ATA_DEV_REMOVABLE 	0x08

#define IS_KNOWN(x)             (x.type & 0x01)
#define IS_ATA(x)               (!(x.type & 0x04))
#define IS_ATAPI(x)     (x.type & 0x04)
#define IS_REMOVABLE(x) (x.type & 0x08)
#define SET_TYPE(x, y)  (x.type |= (y << 4))
#define GET_TYPE(x)             (x.type >> 4)

#define IDE_TYPE_STR(x) (ide_types_str[GET_TYPE(x)])

#define TIMEOUT 1000000 // mikrosekunteina, nyt 1 sec

#define IDE_BYTES_PER_SECTOR 512

typedef struct ide_controller {
	unsigned data;
	unsigned featErr;
	unsigned sectorCount;
	unsigned sectorNumber;
	unsigned cylinderLow;
	unsigned cylinderHigh;
	unsigned driveHead;
	unsigned comStat;
	unsigned altComStat;
} ide_controller_t;

typedef struct ide_device {
	BD_DEVICE blockdev;

	int devnum;
	unsigned char ide_type;
	unsigned char removable;

	char serial_number[21];
	char firmware[9];
	char model[41];
} ide_device_t;

typedef int (*ata_rw_next_sector_t) (uint_t device, uint16_t * buf);

int ide_ascii_rotate(char * str);
int ide_select_device(uint_t device);
int ide_reset(uint_t controller);
void ide_identify_device(uint_t controller, uint_t device);
void ide_probe(void);
int ide_wait (uint_t device);
int ata_read_next_sector (uint_t device, uint16_t * buf);
int ata_write_next_sector (uint_t device, uint16_t * buf);
int ata_identify (uint_t device, void * buf);
int atapi_identify (uint_t device, void * buf);
int setup_LBA (uint_t device, uint64_t sector, uint8_t sector_count);

size_t ata_rw_some (uint_t device, uint64_t sector, uint8_t count, char * buf, ata_rw_next_sector_t rw_next_sector);
size_t ata_rw (uint_t device, uint64_t sector, size_t count, char * buf, ata_rw_next_sector_t rw_next_sector);

size_t ata_read (ide_device_t *device, uint64_t sector, size_t count, void * buf);
size_t ata_write (ide_device_t *device, uint64_t sector, size_t count, void * buf);
int ata_read_one_sector (ide_device_t *device, uint64_t sector, void * buf);
int ata_write_one_sector (ide_device_t *device, uint64_t sector, void * buf);

int ata_safely_remove(ide_device_t *device);

ide_device_t ide_devices[NUM_DEVICES];

extern int ide_init(void);

#endif
