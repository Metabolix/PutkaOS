#ifndef _DIR_H
#define _DIR_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

struct fs;
typedef struct _DIR DIR;
typedef struct _DIRENTRY DIRENTRY;

enum DIR_ERRORS {
	DIR_ERR_TOTAL_FAILURE = -1,
	DIR_ERR_NO_FUNCTIONS = -2,
	DIR_ERR_EXISTS = -3,
	DIR_ERR_CANT_MAKE = -4,

	DIR_ERR__ = -100
};

enum DIRENTRY_TYPES {
	DIRENTRY_UNKNOWN = 0,
	DIRENTRY_FILE,
	DIRENTRY_DIR,
	DIRENTRY_SYMLINK,
	DIRENTRY_PIPE,
	DIRENTRY_DEVICE,
	// TODO: DIRENTRY_TYPES jatkoa

	DIRENTRY_ERROR = -1
};
#define DIRENTRY_CHARDEV DIRENTRY_DEVICE
#define DIRENTRY_BLOCKDEV DIRENTRY_DEVICE

typedef int (*dmake_t)(struct fs *fs, const char * dirname);
typedef DIR *(*dopen_t)(struct fs *fs, const char * dirname);
typedef int (*dread_t)(DIR *listing);
typedef int (*dclose_t)(DIR *listing);

struct dirfunc {
        dmake_t dmake;
        dopen_t dopen;
        dclose_t dclose;
        dread_t dread;
};

struct _DIRENTRY {
	const char *name;
	uint_t uid, gid;
	uint_t rights;
	fpos_t size;
	time_t created, accessed, modified;
	uint_t references;
	uint_t type;
	uint16_t dev_major, dev_minor;
};

struct _DIR {
	DIRENTRY entry;
	const struct dirfunc *func;
};

extern int dmake(const char * dirname);
extern DIR *dopen(const char * dirname);
extern int dread(DIR *listing);
extern int dclose(DIR *listing);

#endif
