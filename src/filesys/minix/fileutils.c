#include <filesys/minix/minix.h>
#include <filesys/minix/fileutils.h>

int minix_link (struct minix_fs *fs, const char *src, const char *dest)
{
        return -1;
}

int minix_symlink (struct minix_fs *fs, const char *src, const char *dest)
{
        return -1;
}

int minix_unlink (struct minix_fs *fs, const char *src)
{
        return -1;
}

int minix_getprops (struct minix_fs *fs, const char *src, struct file_props *val)
{
        return -1;
}

int minix_setprops (struct minix_fs *fs, const char *src, const struct file_props *val)
{
        return -1;
}

