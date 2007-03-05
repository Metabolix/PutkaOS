#ifndef _FAT16_H
#define _FAT16_H 1

#include <filesys/fat.h>

typedef unsigned short fat16_fat_t;

struct fat12_fat_duo {
	unsigned
		e1 : 12,
		e2 : 12;
};

struct fat16_fs {
	/* Yleiset */
	struct fs std;

	/* Omat jutut */
	FILE *device;
	struct fat_header header;
	size_t fat_start;
	size_t rootdir_start;
	size_t data_start;
	size_t data_end;
	size_t cluster_count;
	size_t bytes_per_cluster;
	fat16_fat_t (*get_fat)(struct fat16_fs *fs, fat16_fat_t cluster);
	fat16_fat_t (*set_next_cluster)(struct fat16_fs *fs, fat16_fat_t cluster);
	int (*write_fat)(struct fat16_fs *fs);
	fat16_fat_t fat12_fat[0];
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
	size_t real_first_cluster, real_cluster;
	size_t file_size;
};

#define FAT16_CLUSTER_FREE(fat) ((fat16_fat_t)(fat) == 0)
#define FAT16_CLUSTER_RESERVED(fat) ((fat16_fat_t)(fat) == 1)
#define FAT16_CLUSTER_USED(fat) (((fat16_fat_t)(fat) > 1) && ((fat16_fat_t)(fat) < 0xfff0))
#define FAT16_CLUSTER_BAD(fat) ((fat16_fat_t)(fat) == 0xFFF7)
#define FAT16_CLUSTER_EOF(fat) ((fat16_fat_t)(fat) > 0xFFF7)

extern struct fat16_fs *fat16_mount(FILE *device, uint_t mode, const struct fat_header *fat_header, int fat12);
//extern struct fat16_fs *fat16_mount(FILE *device, uint_t mode, const struct fat_header *fat_header);

fat16_fat_t fat16_get_fat(struct fat16_fs *fs, fat16_fat_t cluster);
fat16_fat_t fat16_set_next_cluster(struct fat16_fs *fs, fat16_fat_t cluster);
fat16_fat_t fat12_get_fat(struct fat16_fs *fs, fat16_fat_t cluster);
fat16_fat_t fat12_set_next_cluster(struct fat16_fs *fs, fat16_fat_t cluster);
int fat12_read_fat(struct fat16_fs *fs);
int fat12_write_fat(struct fat16_fs *fs);

int fat16_umount(struct fat16_fs *this);

void *fat16_fopen_all(struct fat16_fs *this, const char * filename, uint_t mode, int accept_dir);
void *fat16_fopen(struct fat16_fs *this, const char * filename, uint_t mode);
int fat16_fclose(struct fat16_file *stream);

size_t fat16_fread(void *buf, size_t size, size_t count, struct fat16_file *stream);
size_t fat16_fwrite(void *buf, size_t size, size_t count, struct fat16_file *stream);

size_t fat16_fread_rootdir(void *buf, size_t size, size_t count, struct fat16_file *stream);
size_t fat16_fwrite_rootdir(void *buf, size_t size, size_t count, struct fat16_file *stream);

int fat16_fflush(struct fat16_file *stream);

int fat16_fgetpos(struct fat16_file *stream, fpos_t *pos);
int fat16_fsetpos(struct fat16_file *stream, const fpos_t *pos);

int fat16_dmake(struct fat16_fs *this, const char * dirname, uint_t owner, uint_t rights);
struct fat16_dir *fat16_dopen(struct fat16_fs *this, const char * dirname);
int fat16_dread(struct fat16_dir *listing);
int fat16_dclose(struct fat16_dir *listing);

#endif
