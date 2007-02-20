#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H 1
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum MODE_FLAGS {
	FILE_MODE_READ = 1,
	FILE_MODE_WRITE = 2,
	FILE_MODE_APPEND = 4,
	FILE_MODE_CLEAR = 8,
};

struct fs;

typedef void *(*fopen_t)(struct fs *this, const char * filename, uint_t mode);
typedef int (*fclose_t)(void *stream);
typedef size_t (*fread_t)(void *buf, size_t size, size_t count, void *stream);
typedef size_t (*fwrite_t)(void *buf, size_t size, size_t count, void *stream);
typedef int (*fgetpos_t)(void *stream, fpos_t *pos);
typedef int (*fsetpos_t)(void *stream, const fpos_t *pos);
typedef struct fs *(*fs_mount_t)(FILE *device, uint_t mode);
typedef int (*fs_umount_t)(struct fs *this);

struct fs {
	fopen_t fopen;
	fclose_t fclose;
	fread_t fread;
	fwrite_t fwrite;
	fgetpos_t fgetpos;
	fsetpos_t fsetpos;
	fs_mount_t fs_mount;
	fs_umount_t fs_umount;
};

extern struct fs *fs_mount(FILE *dev, uint_t mode);

#endif
