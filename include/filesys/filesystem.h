#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H 1
#include <stddef.h>
#include <stdint.h>
#include <filesys/file.h>
#include <filesys/dir.h>

enum MODE_FLAGS {
	FILE_MODE_READ = 1,
	FILE_MODE_WRITE = 2,
	FILE_MODE_RW = FILE_MODE_READ | FILE_MODE_WRITE,
	FILE_MODE_APPEND = 4,
	FILE_MODE_CLEAR = 8,

	FILE_MODE_ALL = 0x0f // NOTICE: This _must_ be kept to date!
};

struct fs;

typedef struct fs *(*fs_mount_t)(FILE *device, uint_t mode);
typedef int (*fs_umount_t)(struct fs *this);

typedef FILE *(*fopen_t)(struct fs *this, const char * filename, uint_t mode);
typedef int (*fclose_t)(FILE *stream);
typedef size_t (*fread_t)(void *buf, size_t size, size_t count, FILE *stream);
typedef size_t (*fwrite_t)(const void *buf, size_t size, size_t count, FILE *stream);
typedef int (*fflush_t)(FILE *stream);
typedef int (*fsetpos_t)(FILE *stream, const fpos_t *pos);
typedef int (*ioctl_t)(FILE *stream, int request, uintptr_t param);

typedef int (*dmake_t)(struct fs *this, const char * dirname, uint_t owner, uint_t rights);
typedef DIR *(*dopen_t)(struct fs *this, const char * dirname);
typedef int (*dclose_t)(DIR *listing);
typedef int (*dread_t)(DIR *listing);

struct filefunc {
	fopen_t fopen;
	fclose_t fclose;
	fread_t fread;
	fwrite_t fwrite;
	fflush_t fflush;
	fsetpos_t fsetpos;
	ioctl_t ioctl;
};

struct dirfunc {
	dmake_t dmake;
	dopen_t dopen;
	dclose_t dclose;
	dread_t dread;
};

struct fs {
	const char *name;
	fs_mount_t fs_mount;
	fs_umount_t fs_umount;
	struct filefunc filefunc;
	struct dirfunc dirfunc;
};

extern struct fs *fs_mount(FILE *dev, uint_t mode);
extern int fs_add_driver(fs_mount_t mount_function);
extern int fs_init(void);

//extern FILE *fopen(struct fs *this, const char * filename, uint_t mode);
extern int fclose_none(FILE *stream);

extern size_t fread_none(void *buf, size_t size, size_t count, FILE *stream);
extern size_t fwrite_none(void *buf, size_t size, size_t count, FILE *stream);

extern int fflush_none(FILE *stream);

extern int fsetpos_copypos(FILE *stream, const fpos_t *pos);

//extern int success_dmake(struct fs *this, const char * dirname, uint_t owner, uint_t rights);
//extern DIR *success_dopen(struct fs *this, const char * dirname);
//extern int success_dread(DIR *listing);
//extern int success_dclose(DIR *listing);

#endif
