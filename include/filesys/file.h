#ifndef _FILE_H
#define _FILE_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct _FILE {
	fpos_t pos;
	fpos_t size;
	short errno, eof;

	uint_t mode;
	const struct filefunc *func;
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
