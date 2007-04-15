#ifndef _HDD_H
#define _HDD_H 1

#include <storage/blockdev.h>
#include <storage/ide.h>
#include <stdint.h>

enum HDD_PART_TYPES {
	HDD_PART_EMPTY = 0,
	HDD_PART_LOGICAL_DOS = 0x05,
	HDD_PART_LOGICAL_W95 = 0x0f,
	HDD_PART_LOGICAL = 0x85,
	HDD_PART_EFI_GPT = 0xee
};
#define HDD_IS_LOGICAL(x) \
	((x) == HDD_PART_LOGICAL || \
	(x) == HDD_PART_LOGICAL_W95 || \
	(x) == HDD_PART_LOGICAL_DOS)

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

struct _hdd_partition_device_t {
	ide_device_t diskdev;
	ide_device_t *parentdev;
};
typedef struct _hdd_partition_device_t hdd_partdev_t;

extern void hdd_read_partitions(ide_device_t *dev);

#endif
