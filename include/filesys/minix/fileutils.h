#ifndef _MINIX_FILEUTILS_H
#define _MINIX_FILEUTILS_H 1

#include <filesys/minix/minix.h>
#include <filesys/fileutils.h>

extern int minix_link (struct minix_fs *fs, const char *name, const char *linkname);
extern int minix_symlink (struct minix_fs *fs, const char *name, const char *linkname);
extern int minix_unlink (struct minix_fs *fs, const char *name);
extern int minix_getprops (struct minix_fs *fs, const char *name, struct file_props *val);
extern int minix_setprops (struct minix_fs *fs, const char *name, const struct file_props *val);

#endif

