#ifndef _BLOCKDEV_H
#define _BLOCKDEV_H

#include <stddef.h>
#include <filesys/file.h>
#include <devices/devmanager.h>

struct _BD_DEVICE;
struct _BD_FILE;
typedef struct _BD_DEVICE BD_DEVICE;
typedef struct _BD_FILE BD_FILE;

typedef int (*blockdev_refresh_t)(BD_DEVICE *dev);

typedef int (*read_one_block_t)(BD_DEVICE *self, uint64_t num, void * buf);
typedef int (*write_one_block_t)(BD_DEVICE *self, uint64_t num, const void * buf);
typedef size_t (*read_blocks_t)(BD_DEVICE *self, uint64_t num, size_t count, void * buf);
typedef size_t (*write_blocks_t)(BD_DEVICE *self, uint64_t num, size_t count, const void * buf);

struct _BD_DEVICE {
	DEVICE std;

	uint64_t block_size;
	uint64_t block_count;
	uint64_t first_block_num;

	read_one_block_t read_one_block;
	write_one_block_t write_one_block;
	read_blocks_t read_blocks;
	write_blocks_t write_blocks;
	blockdev_refresh_t refresh;
};

#define BLOCKDEV_NO_BLOCK ((uint64_t)(-1))

struct _BD_FILE {
	/* Yleiset */
	FILE std;

	/* Omat jutut */
	struct _BD_DEVICE *dev;

	uint64_t pos_in_block;
	uint64_t block_in_file;

	uint64_t block_in_buffer;
	int buffer_changed;

	char *buf;
};

extern BD_FILE *blockdev_fopen(BD_DEVICE *dev, uint_t mode);
extern int blockdev_fclose(BD_FILE *desc);
extern int blockdev_fflush(BD_FILE *desc);
extern size_t blockdev_fread(void *buffer, size_t size, size_t count, BD_FILE *device);
extern size_t blockdev_fwrite(const void *buffer, size_t size, size_t count, BD_FILE *device);
extern int blockdev_fsetpos(BD_FILE *device, const fpos_t *pos);
extern int blockdev_ioctl(BD_FILE *device, int request, uintptr_t param);

#endif
