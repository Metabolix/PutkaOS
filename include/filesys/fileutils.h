#ifndef _FILEUTILS_H
#define _FILEUTILS_H 1

struct fs;

enum FILE_PROPS {
        FILE_PROP_RIGHTS = 1 << 0,
        FILE_PROP_UID = 1 << 1,
        FILE_PROP_GID = 1 << 2,
};

struct file_props {
        int flags, rights, uid, gid;
};

typedef int (*link_t) (struct fs *fs, const char *src, const char *dest);
typedef int (*unlink_t) (struct fs *fs, const char *file);
typedef int (*getprops_t) (struct fs *fs, const char *file, struct file_props *val);
typedef int (*setprops_t) (struct fs *fs, const char *file, const struct file_props *val);

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

