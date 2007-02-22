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
	MOUNT_ERR_FILESYS_ERROR = -5,

	MOUNT_ERR_FREE_ERRORCODE = -0x10000 // ;)
};

extern int mount_init(const char * root_device);
extern void mount_uninit(void);
extern int mount_something(const char * device_filename, const char * mountpoint, int flags);
extern int umount_something(const char * device_or_point);

extern const struct mountpoint *mount_etsi_kohta(const char ** filename_ptr);

/*
 * Internals
 */
struct mountpoint *etsi_laite_rek(const char * device_name, struct mountpoint *mnt);
struct mountpoint *etsi_kohta(const char ** filename_ptr);
int umount_point(struct mountpoint *mnt);
int umount_array(struct mountpoint *array, size_t size);

#endif
