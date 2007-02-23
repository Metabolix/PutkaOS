#ifndef _FAT12_H
#define _FAT12_H 1

#include <filesys/fat.h>

struct fat16_fs {
	/* Yleiset */
	struct fs std;

	/* Omat jutut */
	FILE *device;
	struct fat_header header;
	unsigned long fat_start;
	unsigned long rootdir_start;
	size_t bytes_per_cluster;
	unsigned short (*get_fat)(struct fat16_fs *fs, int cluster);
	unsigned short fat12_fat[1];
};

struct fat16_dir {
	/* Yleiset */
	DIR std;

	char name[8+3+2];
	struct fat_direntry direntry;
	struct fat16_file *file;
};

struct fat16_file {
	/* Yleisesti tiedosto */
	FILE std;

	/* Omat jutut; tämä on funktiolle FILE */
	struct fat16_fs *fs;
	size_t pos_in, cluster;
	size_t last_pos_in, last_cluster;
	size_t first_cluster_num, cluster_num;
	size_t file_size;
};

#define FAT16_CLUSTER_FREE(fat) (!(fat))
#define FAT16_CLUSTER_RESERVED(fat) ((fat) == 1)
#define FAT16_CLUSTER_USED(fat) (((fat) > 1) && ((fat) < 0xfff0))
#define FAT16_CLUSTER_BAD(fat) ((fat) == 0xFFF7)
#define FAT16_CLUSTER_END(fat) ((fat) == 1)

extern struct fat16_fs *fat12_mount(FILE *device, uint_t mode, const struct fat_header *fat_header);
//extern struct fat16_fs *fat16_mount(FILE *device, uint_t mode, const struct fat_header *fat_header);

unsigned short fat16_get_fat(struct fat16_fs *fs, int cluster);
unsigned short fat12_get_fat(struct fat16_fs *fs, int cluster);

int fat16_umount(struct fat16_fs *this);

void *fat16_fopen(struct fat16_fs *this, const char * filename, uint_t mode);
int fat16_fclose(struct fat16_file *stream);

size_t fat16_fread(void *buf, size_t size, size_t count, struct fat16_file *stream);
size_t fat16_fwrite(void *buf, size_t size, size_t count, struct fat16_file *stream);

int fat16_fflush(struct fat16_file *stream);
long fat16_ftell(struct fat16_file *stream);
int fat16_fseek(struct fat16_file *stream, long int offset, int origin);

int fat16_fgetpos(struct fat16_file *stream, fpos_t *pos);
int fat16_fsetpos(struct fat16_file *stream, const fpos_t *pos);

int fat16_dmake(struct fat16_fs *this, const char * dirname, uint_t owner, uint_t rights);
struct fat16_dir *fat16_dopen(struct fat16_fs *this, const char * dirname);
int fat16_dread(struct fat16_dir *listing);
int fat16_dclose(struct fat16_dir *listing);

#endif
