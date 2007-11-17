#include <filesys/fat/fat.h>
#include <filesys/fat/fat16.h>
#include <memory/kmalloc.h>
#include <string.h>
#include <kprintf.h>

struct fs *fat_mount(FILE *device, uint_t mode)
{
	struct fat_header fat_header;

	fpos_t pos = 0;

	if (fsetpos(device, &pos) || fread(&fat_header, sizeof(fat_header), 1, device) != 1) {
		return 0;
	}
	if (fat_header.total_sectors_small) {
		fat_header.total_sectors = fat_header.total_sectors_small;
	}
	if (memcmp(fat_header.head.fat12.fs_type, FAT12_FS_TYPE, strlen(FAT12_FS_TYPE)) == 0) {
		return (struct fs *) fat16_mount(device, mode, &fat_header, 12);
	}
	if (memcmp(fat_header.head.fat16.fs_type, FAT16_FS_TYPE, strlen(FAT16_FS_TYPE)) == 0) {
		return (struct fs *) fat16_mount(device, mode, &fat_header, 16);
	}
	if (memcmp(fat_header.head.fat32.fs_type, FAT32_FS_TYPE, strlen(FAT32_FS_TYPE)) == 0) {
		return (struct fs *) fat32_mount(device, mode, &fat_header);
	}
	return 0;
}

void fat_mk_struct_tm(unsigned short dateshort, unsigned short timeshort, struct tm *tm)
{
	union fat_time {
		unsigned i;
		struct {
			unsigned
				sec_per_2: 5,
				min: 6,
				hour: 5;
		} parts;
	} time;
	time.i = timeshort;

	union fat_date {
		unsigned i;
		struct {
			unsigned
				day: 5,
				month: 4,
				year: 7;
		} parts;
	} date;
	date.i = dateshort;

	tm->tm_sec = (int)time.parts.sec_per_2 * 2;
	tm->tm_min = (int)time.parts.min;
	tm->tm_hour = (int)time.parts.hour;
	tm->tm_mday = (int)date.parts.day;
	tm->tm_mon = (int)date.parts.month - 1;
	tm->tm_year = (int)date.parts.year + 80;
}

void *fat32_mount(FILE *device, uint_t mode, const struct fat_header *fat_header) {return 0;}
