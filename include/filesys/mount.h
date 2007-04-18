#ifndef _MOUNT_H
#define _MOUNT_H 1

#include <filesys/filesystem.h>
#include <filesys/file.h>
#include <filesys/dir.h>

struct mountpoint {
	char *dev_name;
	char *absolute_path;
	char *relative_path;
	size_t subtree_size;
	struct mountpoint *subtree;
	struct mountpoint *parent;

	FILE *dev;
	struct fs *fs;
};

enum MOUNT_ERR {
	MOUNT_ERR_TOTAL_FAILURE = -1,
	MOUNT_ERR_MOUNTED_SUBPOINTS = -2,
	MOUNT_ERR_BUSY = -3,
	MOUNT_ERR_ALREADY_MOUNTED = -4,
	MOUNT_ERR_DEVICE_ERROR = -5,
	MOUNT_ERR_FILESYS_ERROR = -6,

	MOUNT_ERR_FREE_ERRORCODE = -0x10000 // ;)
};

extern int mount_init(unsigned long mboot_device, const char *mboot_cmdline);
extern void mount_uninit(void);
extern int mount_replace(const char * device_filename, const char * mountpoint, uint_t flags);
extern int mount_something(const char * device_filename, const char * mountpoint, uint_t flags);
extern int umount_something(const char * device_or_point);

extern const struct mountpoint *mount_etsi_kohta(const char ** filename_ptr);

#endif
