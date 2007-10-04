#ifndef _FILE_H
#define _FILE_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct fs;

typedef FILE *(*fopen_t)(struct fs *fs, const char * filename, uint_t mode);
typedef int (*fclose_t)(FILE *stream);
typedef size_t (*fread_t)(void *buf, size_t size, size_t count, FILE *stream);
typedef size_t (*fwrite_t)(const void *buf, size_t size, size_t count, FILE *stream);
typedef int (*fflush_t)(FILE *stream);
typedef int (*fsetpos_t)(FILE *stream, const fpos_t *pos);
typedef int (*ioctl_t)(FILE *stream, int request, uintptr_t param);

enum FILE_MODE_FLAGS {
	FILE_MODE_READ = 1,
	FILE_MODE_WRITE = 2,
	FILE_MODE_RW = FILE_MODE_READ | FILE_MODE_WRITE,
	FILE_MODE_APPEND = 4,
	FILE_MODE_CLEAR = 8,

	FILE_MODE_ALL = 0x0f // NOTICE: This _must_ be kept to date!
};

enum IOCTL_VALUES {
        IOCTL_STD_BEGIN = 0,
        IOCTL_TRUNCATE_FILE = 1,

        IOCTL_STD_END
};

struct _FILE {
	fpos_t pos;
	fpos_t size;
	short errno, eof;

	uint_t mode;
	const struct filefunc *func;
};

struct filefunc {
        fopen_t fopen;
        fclose_t fclose;
        fread_t fread;
        fwrite_t fwrite;
        fflush_t fflush;
        fsetpos_t fsetpos;
        ioctl_t ioctl;
};

extern FILE *fopen_intflags(const char * filename, uint_t flags);
extern int fclose(FILE *stream);
extern size_t fread(void * restrict buf, size_t size, size_t count, FILE * restrict file);
extern size_t fwrite(const void * restrict buf, size_t size, size_t count, FILE * restrict file);
extern int fflush(FILE * restrict stream);
extern int fgetpos(FILE * restrict stream, fpos_t * restrict pos);
extern int fsetpos(FILE * stream, const fpos_t *pos);
extern int ioctl(FILE * stream, int request, uintptr_t param);


#endif
