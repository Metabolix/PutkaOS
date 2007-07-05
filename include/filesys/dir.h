#ifndef _DIR_H
#define _DIR_H 1

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <filesys/file.h>

enum DIR_ERRORS {
	DIR_ERR_TOTAL_FAILURE = -1,
	DIR_ERR_NO_FUNCTIONS = -2,
	DIR_ERR_EXISTS = -3,
	DIR_ERR_CANT_MAKE = -4,
	DIR_ERR_CANT_WRITE = -5,

	DIR_ERR__ = -100
};

typedef struct _DIR {
	char *name;
	fpos_t size;
	uint_t uid, gid;
	uint_t rights;
	time_t created, accessed, modified;
	uint_t references;

	const struct dirfunc *func;
} DIR;

extern int dmake(const char * dirname, uint_t owner, uint_t rights);
extern DIR *dopen(const char * dirname);
extern int dread(DIR *listing);
extern int dclose(DIR *listing);

#endif
