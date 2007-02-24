#include <filesys/fat.h>
#include <filesys/fat16.h>
#include <malloc.h>
#include <string.h>
#include <screen.h>

struct fs *fat_mount(FILE *device, uint_t mode)
{
	struct fat_header fat_header;

	fseek(device, 0, SEEK_SET);
	if (fread(&fat_header, sizeof(fat_header), 1, device) != 1) {
		return 0;
	}
	if (fat_header.total_sectors_small) {
		fat_header.total_sectors = fat_header.total_sectors_small;
	}
	if (memcmp(fat_header.fat12.fs_type, FAT12_FS_TYPE, strlen(FAT12_FS_TYPE)) == 0) {
		return (struct fs *) fat16_mount(device, mode, &fat_header, 12);
	}
	if (memcmp(fat_header.fat16.fs_type, FAT16_FS_TYPE, strlen(FAT16_FS_TYPE)) == 0) {
		return (struct fs *) fat16_mount(device, mode, &fat_header, 16);
	}
	if (memcmp(fat_header.fat32.fs_type, FAT32_FS_TYPE, strlen(FAT32_FS_TYPE)) == 0) {
		return (struct fs *) fat32_mount(device, mode, &fat_header);
	}
	return 0;
}

void *fat32_mount(FILE *device, uint_t mode, const struct fat_header *fat_header) {return 0;}
