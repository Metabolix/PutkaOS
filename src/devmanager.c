#include <devmanager.h>
#include <malloc.h>
#include <string.h>
//#include <screen.h>

struct fs devfs = {
	0,0,
	{
		(fopen_t) dev_fopen,
		0, 0,0,0,0,0
	},
	{
		0,
		(dopen_t) dev_dopen,
		(dclose_t) dev_dclose,
		(dclose_t) dev_dread
	}
};

size_t free_index = 0;
struct device_list devlist = {0, 0};

FILE *dev_fopen(struct fs *this, const char * filename, uint_t mode)
{
	struct device_list *list;
	DEVICE *dev;

	list = &devlist;
	dev = 0;
	while (list) {
		if (list->dev && list->dev->name)
		if (strcmp(filename, list->dev->name) == 0) {
			dev = list->dev;
			break;
		}
		list = list->next;
	}
	if (!dev || !dev->devopen) {
		return 0;
	}
	return dev->devopen(list->dev, mode);
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
		listing->devlist = &devlist;
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
	if (!listing) {
		return -1;
	}
	while (listing->devlist && !listing->devlist->dev) {
		listing->devlist = listing->devlist->next;
	}
	if (!listing->devlist || !listing->devlist->dev) {
		return -1;
	}
	ptr = listing->devlist->dev;
	listing->devlist = listing->devlist->next;
	/* Tiedot diriin... */
	listing->std.name = (char*)ptr->name;
	listing->std.size = 1; // Can we know it?
	listing->std.owner = 0; // Owner... root? xD
	listing->std.rights = 0; // What rights?
	listing->std.created = 0; // Hey, who _creates_ a device?
	listing->std.accessed = 0;
	listing->std.modified = 0;
	listing->std.references = 1; // Only one...
	return 0;
}

int devmanager_init(void)
{
	return 0;
}

int devmanager_uninit(void)
{
	struct device_list *list, *ptr;
	list = &devlist;
	while (list) {
		if (list->dev) {
			list->dev->remove(list->dev);
		}
		list = list->next;
	}
	list = devlist.next;
	while (list) {
		ptr = list;
		list = list->next;
		kfree(ptr);
	}
	return 0;
}

int device_insert(DEVICE *device)
{
	struct device_list *list;

	list = &devlist;
	while (list) {
		if (list->dev && list->dev->name)
		if (strcmp(device->name, list->dev->name) == 0) {
			return DEV_ERR_EXISTS;
		}
		list = list->next;
	}

	list = &devlist;
	while (list->next) {
		if (!list->dev) break;
		list = list->next;
	}
	if (!list->next) {
		list->next = kcalloc(1, sizeof(struct device_list));
	}
	list->dev = device;
	device->index = ++free_index;
	//kprintf("Devmanager: Added device '%s' with index %d\n", device->name, device->index);
	return 0;
}
