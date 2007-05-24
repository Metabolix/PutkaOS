#include <filesys/pseudofsdriver.h>
#include <filesys/fat.h>
#include <filesys/filesystem.h>
#include <filesys/ext2.h>
#include <malloc.h>
#include <string.h>
#include <list.h>

#define MAX_NAME_LEN 15

LIST_TYPE(fs_driver, fs_mount_t);
list_of_fs_driver fs_driver_list;

struct fs *fs_mount(FILE *dev, uint_t mode)
{
	struct fs *retval;
	list_iter_of_fs_driver iter;

	/* Valitaan listasta oikea juttu */
	list_loop(iter, fs_driver_list) {
		if ((retval = list_item(iter)(dev, mode))) {
			return retval;
		}
	}
	return 0;
}

int fs_add_driver(fs_mount_t mount_function)
{
	return list_insert(list_end(fs_driver_list), mount_function);
}

int fs_init(void)
{
	list_init(fs_driver_list);
	return fs_add_driver(ext2_mount) + fs_add_driver(fat_mount);
}
