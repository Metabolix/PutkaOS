#ifndef _MOUNT_H
#define _MOUNT_H 1

#include <filesys/filesystem.h>
#include <filesys/file.h>
#include <filesys/dir.h>
#include <filesys/mount_err.h>

struct mount {
	char *dev_name;
	char *absolute_path;
	char *relative_path;
	size_t subtree_size;
	struct mount *subtree;
	struct mount *parent;

	FILE *dev;
	struct fs *fs;
};

extern int mount_init(uint8_t mboot_device[4], const char *mboot_cmdline);
extern void mount_uninit(void);
extern int mount_replace(const char * device_filename, const char * mountpoint, uint_t flags);
extern int mount_something(const char * device_filename, const char * mountpoint, uint_t flags);
extern int umount_something(const char * device_or_point);

typedef void (*mount_foreach_func_t)(const char *fs_name, const char *dev_name, const char *absolute_path, const char *relative_path, int level);
extern int mount_count(void);
extern int mount_foreach(mount_foreach_func_t f);

extern const struct mount *mount_etsi_kohta(const char ** filename_ptr);

#endif
