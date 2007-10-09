#ifndef _FILEUTILS_H
#define _FILEUTILS_H 1

struct fs;

#include <filesys/dir.h>

enum FILE_PROPS {
        FILE_PROP_TYPE = 1 << 0,
        FILE_PROP_RIGHTS = 1 << 1,
        FILE_PROP_UID = 1 << 2,
        FILE_PROP_GID = 1 << 3,
};

struct file_props {
        uint_t flags;
        uint_t type; // DIRENTRY_... @ dir.h
        uint_t rights;
        uint_t uid, gid;
};

typedef int (*link_t) (struct fs *fs, const char *name, const char *linkname);
typedef int (*unlink_t) (struct fs *fs, const char *name);
typedef int (*getprops_t) (struct fs *fs, const char *name, struct file_props *val);
typedef int (*setprops_t) (struct fs *fs, const char *name, const struct file_props *val);

struct fileutils {
        link_t link, symlink;
        unlink_t unlink;
        getprops_t getprops;
        setprops_t setprops;
};

extern int link (const char *src, const char *dest);
extern int symlink (const char *src, const char *dest);
extern int unlink (const char *src);
extern int getprops (const char *src, struct file_props *val);
extern int setprops (const char *src, const struct file_props *val);

#endif

