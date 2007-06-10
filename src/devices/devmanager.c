#include <devices/devmanager.h>
#include <devices/specialdevs.h>
#include <malloc.h>
#include <string.h>
#include <list.h>
#include <ctype.h>
//#include <screen.h>

LIST_TYPE(device, DEVICE*);

list_of_device devlist;

struct devfs_dir {
	DIR std;
	list_iter_of_device next;
};

struct fs devfs = {
	.name = "devfs",
	.fs_mount = 0,
	.fs_umount = 0,
	.filefunc = {
		.fopen = (fopen_t) dev_fopen,
	},
	.dirfunc = {
		.dopen = (dopen_t) dev_dopen,
		.dclose = (dclose_t) dev_dclose,
		.dread = (dread_t) dev_dread
	}
};

size_t free_index = 0;

FILE *dev_fopen(struct fs *this, const char * filename, uint_t mode)
{
	DEVICE *dev;
	list_iter_of_device i;

	dev = 0;
	list_loop(i, devlist) {
		if (list_item(i)->name)
		if (strcmp(filename, list_item(i)->name) == 0) {
			dev = list_item(i);
			break;
		}
	}
	if (!dev || !dev->devopen) {
		return 0;
	}
	return dev->devopen(dev, mode);
}

struct devfs_dir *dev_dopen(struct fs *this, const char * dirname)
{
	/* Emme tue hakemistoja.
	int dev_dmake(struct fs *this, const char * dirname, uint_t owner, uint_t rights); */
	if (*dirname) {
		return 0;
	}
	struct devfs_dir *listing = kmalloc(sizeof(struct devfs_dir));
	if (listing) {
		listing->std.func = &devfs.dirfunc;
		listing->next = list_begin(devlist);
		return listing;
	}
	return 0;
}

int dev_dclose(struct devfs_dir *listing)
{
	if (listing) {
		kfree(listing);
		return 0;
	}
	return -1;
}

int dev_dread(struct devfs_dir *listing)
{
	DEVICE *ptr;
	if (!listing || !list_next(listing->next)) {
		return -1;
	}

	ptr = list_item(listing->next);

	/* Tiedot diriin... */
	listing->std.name = (char*)ptr->name;
	listing->std.size = 1; // Can we know it?
	listing->std.uid = 0; // Owner... root? xD
	listing->std.gid = 0; // Owner... root? xD
	listing->std.rights = 0; // What rights?
	listing->std.created = 0; // Hey, who _creates_ a device?
	listing->std.accessed = 0;
	listing->std.modified = 0;
	listing->std.references = 1; // Only one...

	list_inc(listing->next);
	return 0;
}

int devmanager_init(void)
{
	list_init(devlist);
	special_devices_init();
	return 0;
}

int devmanager_uninit(void)
{
	list_iter_of_device i;
	list_loop(i, devlist) {
		list_item(i)->remove(list_item(i));
	}
	list_destroy(devlist);
	return 0;
}

int device_insert(DEVICE *device)
{
	list_iter_of_device i;

	list_loop(i, devlist) {
		if (strcmp(device->name, list_item(i)->name) == 0) {
			return DEV_ERR_EXISTS;
		}
	}

	if (list_insert(list_end(devlist), device)) {
		return DEV_ERR_TOTAL_FAILURE;
	}
	device->index = ++free_index;
	//kprintf("Devmanager: Added device '%s' with index %d\n", device->name, device->index);
	return 0;
}

#if 0
int device_fill_remove(DEVICE * device) {
		int ret = device->real_remove(device);
		kfree((void*)device->name);
		return ret;
}

int device_fill(DEVICE *devices, dev_class_t class, dev_type_t type, const char *devname, devopen_t devopen, devrm_t remove, size_t count)
{
	char * name = strdup(devname);
	char * new_name;
	int name_len = strlen(name) - 1;
	int nums = 1;
	int inserted = 0;

	if (!isdigit(name[name_len])) {
		kfree(name);
		return -1;
	}

	if (isdigit(name[name_len]) && isdigit(name[name_len - 1])) {
		//int number =  (name[name_len - 1] - '0') * 10 - name[name_len] - '0';
		nums++;
		name_len--;
	}

	while (count--) {
		devices->dev_class = class;
		devices->dev_type = type;
		devices->devopen = devopen;
		devices->remove = device_fill_remove;
		devices->real_remove = remove;
		devices->name = name;

		if (device_insert(devices) == 0) {
			inserted++;
		}

		devices++;

		if (name[name_len + nums - 1] < '9') {
			//new_name = strdup(name);
			new_name = kmalloc(name_len + nums + 1);
			strncpy(new_name, name, name_len + nums + 1);
			new_name[name_len + nums - 1]++;
			name = new_name;
		} else if (name[name_len + nums - 1] == '9') {
			if (nums == 1) {
				nums++;
			}

			if (name[name_len + nums - 1] == '9' && name[name_len + nums - 2] == '9') {
				return inserted;
			}

			new_name = kmalloc(name_len + nums + 1);
			strncpy(new_name, name, name_len + nums + 1);
			name = new_name;
			if (name[name_len + nums - 2] == '9') {
				name[name_len + nums - 2] = '1';
			} else {
				name[name_len + nums - 2]++;
			}
			name[name_len + nums - 1] = '0';
		}
	}

	return inserted;
}
#endif
