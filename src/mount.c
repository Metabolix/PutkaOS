#include <mount.h>
#include <devmanager.h>
#include <filesystem.h>
#include <kmalloc.h>

#define MAKE_NULL(b, a); {if (a) { b(a); (a) = 0; }}

struct mountpoint root = {
	"/",
	"/",
	0,
	0, 0,
	0, 0
};

struct mountpoint *etsi_kohta(const char ** filename_ptr)
{
	int i;
	char *filename = *filename_ptr;
	char *newfilename;
	struct mountpoint *mnt = &root;

	if (filename[0] == '/') {
		loytyi = 1;
		++filename;
		if (filename[0] == 0) {
			return mnt;
		}
	} else {
		return 0;
	}
silmukka:
	for (i = 0; i < mnt->subtree_size; ++i) {
		newfilename = filename;
		j = strrmsame(&newfilename, mnt->subtree[i].relative_point);

		if (mnt->subtree[i].relative_point[j] == 0) {
			if (newfilename[0] == 0) {
				mnt = mnt->subtree[i];
				filename = newfilename;
				goto oikea_paikka;
			}
			if (newfilename[0] == '/') {
				mnt = mnt->subtree[i];
				filename = newfilename + 1;
				if (newfilename[1] == 0) {
					goto oikea_paikka;
				}
				goto silmukka;
			}
		}
	}
oikea_paikka:
	*filename_ptr = filename;
	return mnt;
}

int init_mount(const char * restrict root_device)
{
	root.dev = dev_fopen(root_device, "w+");
	if (!root.dev) {
		root.dev = dev_fopen(root_device, "r");
	}
	if (!root.dev) {
		kprintf("Mount: Couldn't open root device (%s).\n", root_device);
		/* TODO: Creating ramfs instead...\n */
		return -1;
	}

	root.fs = avaa_fs(root.dev);
	if (!root.fs) {
		kprintf("Mount: Couldn't mount root device (%s).\n", root_device);
		return -1;
	}
	if (!root.fs.fwrite) {
		kprintf("Mount: Warning: root device (%s) mounted read-only!\n", root_device);
	}

	root.dev_name = kmalloc(strlen(root_device) + 1);
	if (!root.dev_name) {
		panic("Mount: Out of kernel memory!\n");
	}
	memcpy(root.dev_name, root_device, strlen(root_device) + 1);

	root.subtree_size = 1;
	root.subtree = kmalloc(sizeof(struct mountpoint));
}

int umount_array(struct mountpoint *array, size_t size)
{
	int i = 0;
	if (array) while (size) {
		--size;
		if (umount_array(array[size].subtree, array[size].subtree_size)) {
			i = -1;
		} else {
			MAKE_NULL(kfree, array[size].subtree);
			array[size].subtree_size = 0;

			if (umount_point(array + size)) {
				i = -1;
			}
		}
	}
	return i;
}

int uninit_mount(void)
{
	int i, j = 1;
	if (j) j = umount_array(root.subtree, root.subtree_size);
	if (j) j = umount_array(root.subtree, root.subtree_size);
	if (j) j = umount_array(root.subtree, root.subtree_size);
	if (j) {
		kprintf("Mount: uninit: warning: not everything was umounted.\n");
	}
	root.subtree_size = 0;
	MAKE_NULL(kfree, root.subtree);
	MAKE_NULL(kfree, root.dev_name);
	if (i = root.fs->umount(root.fs)) {
		kprintf("Mount: Umount: umount failed at '%s' (%s)\n", root.absolute_point, point->dev);
		return i;
	}
	if (i = root.dev->fs->fclose(root.dev)) {
		kprintf("Mount: Umount: fclose failed (WTF? :D) at device '%s' (%s)\n", root.dev, root.absolute_point);
		return i;
	}
	return 0;
}

int mount_point(const char * restrict device_filename, const char * restrict mountpoint, const char * restrict mode)
{
	struct mount *mnt;

	mnt = etsi_kohta(&mountpoint);
}

int umount_point(struct mountpoint *point)
{
	int i;
	if (i = point->fs->umount(point->fs)) {
		kprintf("Mount: Umount: umount failed at '%s' (%s)\n", point->absolute_point, point->dev);
		return i;
	}
	if (i = point->dev->fs->fclose(point->dev)) {
		kprintf("Mount: Umount: fclose failed (WTF? :D) at device '%s' (%s)\n", point->dev, point->absolute_point);
		return i;
	}
	return 0;
}

int umount_something(const char * restrict device_or_point)
{
	struct mount *mnt;
	const char *filename;
	int i;

	filename = device_or_point;
	mnt = etsi_kohta(&filename);
	if (filename[0] == 0) {
		if (mnt->subtree_size) {
			return MOUNT_ERR_MOUNTED_SUBPOINTS;
		}
		switch (i = mnt->fs->umount(mnt->fs)) {
			case MOUNT_ERR_BUSY:
				return MOUNT_ERR_BUSY;
			case 0:
				MAKE_NULL(kfree, mnt->relative_point);
				MAKE_NULL(kfree, mnt->absolute_point);
				MAKE_NULL(kfree, mnt->dev_name);
				mnt->subtree_size = 0;
				MAKE_NULL(kfree, mnt->subtree);
				MAKE_NULL(fclose, mnt->dev);
				mnt->fs = 0;
				return 0;
			default:
				kprintf("Mount: Umounting %s failed! Just told us %d... F*cking fs driver!\n", device_or_point, i);
				return MOUNT_ERR_TOTAL_FAILURE;
			}
		}
	}
	kprintf("Mount: Umount: %s isn't a point (can't umount devices yet)\n", device_or_point);
	return MOUNT_ERR_TOTAL_FAILURE;
}
