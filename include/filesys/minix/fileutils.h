#ifndef _MINIX_FILEUTILS_H
#define _MINIX_FILEUTILS_H 1

#include <filesys/minix/minix.h>
#include <filesys/fileutils.h>

extern int minix_link (struct minix_fs *fs, const char *src, const char *dest);
extern int minix_symlink (struct minix_fs *fs, const char *src, const char *dest);
extern int minix_unlink (struct minix_fs *fs, const char *src);
extern int minix_getprops (struct minix_fs *fs, const char *src, struct file_props *val);
extern int minix_setprops (struct minix_fs *fs, const char *src, const struct file_props *val);

#endif

