#ifndef _DIR_H
#define _DIR_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

struct fs;
typedef struct _DIR DIR;

enum DIR_ERRORS {
	DIR_ERR_TOTAL_FAILURE = -1,
	DIR_ERR_NO_FUNCTIONS = -2,
	DIR_ERR_EXISTS = -3,
	DIR_ERR_CANT_MAKE = -4,
	DIR_ERR_CANT_WRITE = -5,

	DIR_ERR__ = -100
};

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

struct _DIR {
	char *name;
	fpos_t size;
	uint_t uid, gid;
	uint_t rights;
	time_t created, accessed, modified;
	uint_t references;

	const struct dirfunc *func;
};

extern int dmake(const char * dirname);
extern DIR *dopen(const char * dirname);
extern int dread(DIR *listing);
extern int dclose(DIR *listing);

#endif
