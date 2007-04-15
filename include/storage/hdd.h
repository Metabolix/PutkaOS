#ifndef _HDD_H
#define _HDD_H 1

#include <storage/blockdev.h>
#include <storage/ide.h>
#include <stdint.h>

struct hdd_bootrec_entry {
	uint32_t
		nulls :7,
		status :1,
		start_chs :24;
	uint32_t
		type :8,
		end_chs :24;
	uint32_t
		start_lba,
		sector_count;
}
__attribute__((packed));

#define HDD_BOOTREC_SKIP (512-sizeof(struct hdd_bootrec))
struct hdd_bootrec {
	//char code_area[394]; // Maybe even 446
	//char IBM_specific_partition_table[36];
	//char optional_disk_signature[4];
	//char some_more_shit[2];
	struct hdd_bootrec_entry osio[4];
	uint16_t xAA55;
}
__attribute__((packed));

struct _hdd_partition_t {
	BD_DEVICE blockdev;
	ide_device_t *diskdev;
};

extern void hdd_read_partitions(ide_device_t *dev);

#endif
