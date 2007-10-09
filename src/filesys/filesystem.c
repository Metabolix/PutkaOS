#include <filesys/filesystem.h>
#include <malloc.h>
#include <string.h>
#include <list.h>

#include <filesys/minix/minix.h>
#include <filesys/ext2/ext2.h>
#include <filesys/fat/fat.h>

LIST_TYPE(fs_driver, fs_mount_t);
list_of_fs_driver fs_driver_list;

struct fs *fs_mount(FILE *dev, uint_t mode)
{
	struct fs *retval;
	list_iter_of_fs_driver iter;

	/* Valitaan listasta oikea juttu */
	list_loop(iter, fs_driver_list) {
		if ((retval = list_item(iter)(dev, mode))) {
			if (!retval->mode) {
				retval->mode = mode;
			}
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
	return 0
		+ fs_add_driver(minix_mount)
		+ fs_add_driver(ext2_mount)
		+ fs_add_driver(fat_mount);
}



int fclose_none(FILE *stream)
{
	return 0;
}

size_t fread_none(void *buf, size_t size, size_t count, FILE *stream)
{
	return size * count;
}

size_t fwrite_none(void *buf, size_t size, size_t count, FILE *stream)
{
	return size * count;
}

int fflush_none(FILE *stream)
{
	return 0;
}

int fsetpos_copypos(FILE *stream, const fpos_t *pos)
{
	stream->pos = *pos;
	return 0;
}
