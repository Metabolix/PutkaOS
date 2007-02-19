#ifndef _MOUNT_H
#define _MOUNT_H 1

#include <filesystem.h>
#include <stdio.h>

struct mountpoint {
	char *relative_point;
	char *absolute_point;
	char *dev_name;
	size_t subtree_size;
	struct mountpoint *subtree;

	FILE *dev;
	struct fs *fs;
};

enum MOUNT_ERR {
	MOUNT_ERR_TOTAL_FAILURE = -1,
	MOUNT_ERR_MOUNTED_SUBPOINTS = -2,
	MOUNT_ERR_BUSY = -3,

	MOUNT_ERR_FREE_ERRORCODE = -128; // ;)
};

extern int init_mount(const char * restrict root_device);
extern struct mountpoint *etsi_kohta(const char ** filename_ptr);

#endif
