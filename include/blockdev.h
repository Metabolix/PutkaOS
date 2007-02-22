#ifndef _BLOCKDEV_H
#define _BLOCKDEV_H

#include <stddef.h>
#include <filesys/file.h>
#include <devmanager.h>

struct _BD_DEVICE;
struct _BD_FILE;

typedef int (*read_block_t)(struct _BD_DEVICE *self, size_t num, void * buf);
typedef int (*write_block_t)(struct _BD_DEVICE *self, size_t num, const void * buf);

struct _BD_DEVICE {
	DEVICE std;

	size_t block_size;
	size_t block_count;

	read_block_t read_block;
	write_block_t write_block;
};

struct _BD_FILE {
	/* Yleiset */
	FILE std;

	/* Omat jutut */
	struct _BD_DEVICE *phys;

	size_t pos_in_block;
	size_t block_in_dev;

	unsigned has_read :1, has_written:1;
	char *buffer;
};

typedef struct _BD_DEVICE BD_DEVICE;
typedef struct _BD_FILE BD_FILE;

extern BD_FILE *blockdev_fopen(BD_DEVICE *dev, uint_t mode);
extern void blockdev_fclose(BD_FILE *desc);
extern void blockdev_fflush(BD_FILE *desc);
extern size_t blockdev_fread(void *buffer, size_t size, size_t count, BD_FILE *device);
extern size_t blockdev_fwrite(const void *buffer, size_t size, size_t count, BD_FILE *device);
extern int blockdev_fseek(BD_FILE *device, long int offset, int origin);
extern long blockdev_ftell(BD_FILE *device);
extern int blockdev_fgetpos(BD_FILE *device, fpos_t *pos);
extern int blockdev_fsetpos(BD_FILE *device, const fpos_t *pos);

#endif
