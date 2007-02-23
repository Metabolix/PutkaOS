#ifndef _FAT_H
#define _FAT_H 1

#include <filesys/filesystem.h>

struct fat12_ext_header {
	unsigned char phys_drive_number;
	unsigned char reserved__current_head;
	unsigned char signature;
	unsigned long serial_number;
	char volume_label[11];
	char fs_type[8];
} __attribute__((packed));

#define fat16_ext_header fat12_ext_header

struct fat32_ext_header {
	unsigned long sectors_per_fat;
	unsigned short fat_flags;
	unsigned short version;
	unsigned long rootdir_start_cluster;
	unsigned short fs_information_sector;
	unsigned short boot_sector_copy;
	char reserved_12[12];

	unsigned char phys_drive_number;
	unsigned char reserved__current_head;
	unsigned char signature;
	unsigned long serial_number;
	char volume_label[11];
	char fs_type[8];
} __attribute__((packed));

struct fat_header {
	char jmp_instruction[3];
	char oem_name[8];
	unsigned short bytes_per_sector;
	unsigned char sectors_per_cluster;
	unsigned short reserved_sectors;
	unsigned char number_of_fats;
	unsigned short max_rootdir_entries;
	unsigned short total_sectors_small;
	unsigned char media_desc;
	unsigned short sectors_per_fat;
	unsigned short sectors_per_track;
	unsigned short number_of_heads;
	unsigned long hidden_sectors;
	unsigned long total_sectors;
	union {
		struct fat32_ext_header fat32;
		struct fat16_ext_header fat16;
		struct fat12_ext_header fat12;
	};
} __attribute__((packed));

struct fat_direntry {
	char basename[8];
	char extension[3];
	unsigned char attributes;
	unsigned char reserved;
	unsigned char create_time_10ms;
	unsigned short create_time;
	unsigned short create_date;
	unsigned short access_date;
	union {
		unsigned short ea_index;
		unsigned short first_cluster_hiword;
	};
	unsigned short modify_time;
	unsigned short modify_date;
	unsigned short first_cluster;
	unsigned long file_size;
} __attribute__((packed));

struct fat_lfnentry {
	char seq_num;
	char name1[10];
	unsigned char attributes_0x0f;
	unsigned char reserved_0x00;
	unsigned char checksum;
	char name2[12];
	unsigned short first_cluster_0x0000;
	char name3[4];
} __attribute__((packed));

#define FAT12_FS_TYPE "FAT12 "
#define FAT16_FS_TYPE "FAT16 "
#define FAT32_FS_TYPE "FAT32 "

typedef enum {
	FAT_TYPE_FAT12,
	FAT_TYPE_FAT16,
	FAT_TYPE_FAT32
} fat_fs_type_t;

enum FAT_FILE_ATTRIBUTES {
	FAT_ATTR_READONLY = 0x01,
	FAT_ATTR_HIDDEN = 0x02,
	FAT_ATTR_SYSTEM = 0x04,
	FAT_ATTR_VOLUME_LABEL = 0x08,
	FAT_ATTR_SUBDIR = 0x10,
	FAT_ATTR_ARCHIVE = 0x20,
	FAT_ATTR_DEVICE = 0x40,
	//FAT_ATTR_UNUSED = 0x80
};

extern struct fs *fat_mount(FILE *device, uint_t mode);

// Huom, yksikään ei ole extern
void *fat32_mount(FILE *device, uint_t mode, const struct fat_header *fat_header);
void *fat16_mount(FILE *device, uint_t mode, const struct fat_header *fat_header);

#endif
