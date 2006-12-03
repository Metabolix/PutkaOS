#ifndef _BLOCKDEV_H
#define _BLOCKDEV_H

typedef enum {
	DEV_TYPE_NONE = 0,
	DEV_TYPE_FLOPPY = 1,
	DEV_TYPE_ERROR = 0xffffffff,
} dev_type_t;

enum __ENUM_SEEK {
	SEEK_SET = 0,
	SEEK_CURRENT = 0,
	SEEK_SET = 0,
};

typedef struct __BLOCK_DEVICE {
	dev_type_t dev_type;
	char *name;
	size_t index;
	size_t block_size;
	size_t block_count;
	int (*read_block)(size_t num, void * buf);
	int (*write_block)(size_t num, void * buf);
} BLOCK_DEVICE;

typedef struct __BLOCK_DEVICE_DESCRIPTOR {
	size_t pos;
	size_t bufsize;
	char *buffer;
} BLOCK_DEVICE_DESCRIPTOR;
typedef BLOCK_DEVICE_DESCRIPTOR BD_DESC;

extern BD_DESC *dopen(BLOCK_DEVICE *dev);
extern void dclose(BD_DESC *desc);
extern void dflush(BD_DESC *desc);

#endif
