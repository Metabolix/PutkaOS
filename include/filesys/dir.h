#ifndef _DIR_H
#define _DIR_H 1

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <filesys/file.h>

typedef struct _DIR {
	char *name;
	fpos_t size;
	uint_t owner;
	uint_t rights;
	time_t created, accessed, modified;
	uint_t references;

	struct dirfunc *func;
} DIR;

extern int dmake(const char * dirname, uint_t owner, uint_t rights);
extern DIR *dopen(const char * dirname);
extern int dread(DIR *listing);
extern int dclose(DIR *listing);

#endif
