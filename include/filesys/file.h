#ifndef _FILE_H
#define _FILE_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <pos/file.h>

struct fs;

typedef FILE *(*fopen_t)(struct fs *fs, const char * filename, uint_t mode);
typedef int (*fclose_t)(FILE *stream);
typedef size_t (*fread_t)(void *buf, size_t size, size_t count, FILE *stream);
typedef size_t (*fwrite_t)(const void *buf, size_t size, size_t count, FILE *stream);
typedef int (*fflush_t)(FILE *stream);
typedef int (*fsetpos_t)(FILE *stream, const fpos_t *pos);
typedef int (*ioctl_t)(FILE *stream, int request, intptr_t param);

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

#endif
