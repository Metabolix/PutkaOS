#ifndef _BLOCKDEV_H
#define _BLOCKDEV_H

#include <stddef.h>

#define EOD -1

typedef enum {
	DEV_TYPE_NONE = 0,
	DEV_TYPE_FLOPPY = 1,
	DEV_TYPE_ERROR = 0xffffffff,
} dev_type_t;

enum __ENUM_SEEK {
	SEEK_SET = 0,
	SEEK_CUR = 1,
	SEEK_END = 2,
};

typedef struct __BLOCK_DEVICE {
	dev_type_t dev_type;
	char *name;
	size_t index;
	size_t block_size;
	size_t block_count;
	int (*read_block)(struct __BLOCK_DEVICE *self, size_t num, void * buf);
	int (*write_block)(struct __BLOCK_DEVICE *self, size_t num, const void * buf);
} BLOCK_DEVICE;

typedef struct __BLOCK_DEVICE_DESCRIPTOR {
	BLOCK_DEVICE *phys;
	size_t block_in_dev, pos_in_block;
	unsigned has_read : 1, has_written : 1;
	char *buffer;
} BLOCK_DEVICE_DESCRIPTOR;
typedef BLOCK_DEVICE_DESCRIPTOR BD_DESC;

extern BD_DESC *dopen(BLOCK_DEVICE *dev);
extern void dclose(BD_DESC *desc);
extern void dflush(BD_DESC *desc);
extern int dread(void *buffer, size_t size, size_t count, BD_DESC *device);
extern int dwrite(const void *buffer, size_t size, size_t count, BD_DESC *device);
extern int dseek(BD_DESC *device, long offset, int origin);
extern long dtell(BD_DESC *device);

#endif
