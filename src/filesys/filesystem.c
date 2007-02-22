#include <filesys/pseudofsdriver.h>
#include <filesys/filesystem.h>
#include <malloc.h>

#define MAX_FS_DRIVERS 16

fs_mount_t list_of_fs_mount[MAX_FS_DRIVERS] = {
	(fs_mount_t) pfs_mount,
	0 /* Terminator */
};

struct fs *fs_mount(FILE *dev, uint_t mode)
{
	int i;
	struct fs *retval;

	/* Valitaan listasta oikea juttu */
	for (i = 0; i < MAX_FS_DRIVERS && list_of_fs_mount[i]; ++i) {
		if ((retval = list_of_fs_mount[i](dev, mode))) {
			return retval;
		}
	}
	return 0;
}