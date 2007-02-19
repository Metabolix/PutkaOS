#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H 1
#include <stddef.h>
#include <stdio.h>

struct fs;

typedef void *(*fopen_t)(struct fs *this, const char * restrict filename, const char * restrict mode);
typedef int (*fclose_t)(void *stream);
typedef size_t (*fread_t)(void *buf, size_t size, size_t count, void *stream);
typedef size_t (*fwrite_t)(void *buf, size_t size, size_t count, void *stream);
typedef int (*fgetpos_t)(void *stream, fpos_t *pos);
typedef int (*fsetpos_t)(void *stream, const fpos_t *pos);
typedef struct fs *(*fs_mount_t)(FILE *device, unsigned int mode);
typedef int (*fs_umount_t)(struct fs *this);

struct fs {
	fopen_t fopen;
	fclose_t fclose;
	fread_t fread;
	fwrite_t fwrite;
	fgetpos_t fgetpos;
	fsetpos_t fsetpos;
	fs_umount_t fs_umount;
};

#endif
