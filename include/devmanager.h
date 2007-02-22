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
	DEV_TYPE_ERROR = 0xffffffff
} dev_type_t;


enum DEV_INSERT_ERROR {
	DEV_ERR_TOTAL_FAILURE = -1,
	DEV_ERR_BAD_NAME = -2,
	DEV_ERR_EXISTS = -3,
	DEV_ERR_BAD_STRUCT = -4,

	DEV_ERR_FREE_ERRORCODE = -0x10000
};

struct DEVICE_ENTRY;

typedef FILE *(*devopen_t)(struct DEVICE_ENTRY *device, uint_t mode);
typedef int (*devrm_t)(struct DEVICE_ENTRY *device);

struct DEVICE_ENTRY {
	const char *name;
	dev_class_t dev_class;
	dev_type_t dev_type;
	size_t index;
	devopen_t devopen;
	devrm_t remove;
};
typedef struct DEVICE_ENTRY DEVICE;

extern struct fs devfs;
extern int devmanager_init(void);
extern int device_insert(struct DEVICE_ENTRY *device);
extern int devmanager_uninit(void);

/*
 * Internals
 */
FILE *dev_fopen(struct fs *this, const char * filename, uint_t mode);
int dev_fclose(FILE *stream);

int dev_dmake(struct fs *this, const char * dirname, uint_t owner, uint_t rights);
DIR *dev_dopen(struct fs *this, const char * dirname);
int dev_dread(DIR *listing);
int dev_dclose(DIR *listing);

#endif
