#ifndef _DEVMANAGER_H
#define _DEVMANAGER_H 1

#include <filesys/filesystem.h>
#include <stdint.h>

typedef enum {
	DEV_CLASS_NONE = 0,
	DEV_CLASS_OTHER = 1,
	DEV_CLASS_BLOCK = 2,
	DEV_CLASS_ERROR = 0xffffffff
} dev_class_t;

typedef enum {
	DEV_TYPE_NONE = 0,
	DEV_TYPE_OTHER = 1,
	DEV_TYPE_FLOPPY = 2,
	DEV_TYPE_HARDDISK = 3,
	DEV_TYPE_CDROM = 4,
	DEV_TYPE_HARDDISK_REMOVABLE = 5,
	DEV_TYPE_ERROR = 0xffffffff
} dev_type_t;


enum DEV_INSERT_ERROR {
	DEV_ERR_TOTAL_FAILURE = -1,
	DEV_ERR_BAD_NAME = -2,
	DEV_ERR_EXISTS = -3,
	DEV_ERR_BAD_STRUCT = -4,

	DEV_ERR_FREE_ERRORCODE = -0x10000
};

struct device;
struct devfs_dir;

typedef struct device DEVICE;

typedef FILE *(*devopen_t)(DEVICE *device, uint_t mode);
typedef int (*devrm_t)(DEVICE *device);

struct device {
	const char *name;
	dev_class_t dev_class;
	dev_type_t dev_type;
	size_t index; // device_insert sets this.
	devopen_t devopen;
	devrm_t remove;
};

extern struct fs devfs;
extern int devmanager_init(void);
extern int device_insert(struct device *device);
extern int devmanager_uninit(void);

/*
 * Internals
 */
FILE *dev_fopen(struct fs *this, const char * filename, uint_t mode);

struct devfs_dir *dev_dopen(struct fs *this, const char * dirname);
int dev_dread(struct devfs_dir *listing);
int dev_dclose(struct devfs_dir *listing);

#endif
