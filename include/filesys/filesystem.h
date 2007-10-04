#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H 1

#include <stddef.h>
#include <stdint.h>

struct fs;

#include <filesys/file.h>
#include <filesys/dir.h>
#include <filesys/fileutils.h>

typedef struct fs *(*fs_mount_t)(FILE *device, uint_t mode);
typedef int (*fs_umount_t)(struct fs *fs);

struct fs {
	const char *name;
	fs_mount_t fs_mount;
	fs_umount_t fs_umount;
	struct filefunc filefunc;
	struct dirfunc dirfunc;
	struct fileutils fileutils;
};

extern struct fs *fs_mount(FILE *dev, uint_t mode);
extern int fs_add_driver(fs_mount_t mount_function);
extern int fs_init(void);

//extern FILE *fopen(struct fs *fs, const char * filename, uint_t mode);
extern int fclose_none(FILE *stream);

extern size_t fread_none(void *buf, size_t size, size_t count, FILE *stream);
extern size_t fwrite_none(void *buf, size_t size, size_t count, FILE *stream);

extern int fflush_none(FILE *stream);

extern int fsetpos_copypos(FILE *stream, const fpos_t *pos);

//extern int success_dmake(struct fs *fs, const char * dirname, uint_t owner, uint_t rights);
//extern DIR *success_dopen(struct fs *fs, const char * dirname);
//extern int success_dread(DIR *listing);
//extern int success_dclose(DIR *listing);

#endif
