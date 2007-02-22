#include <devmanager.h>
#include <malloc.h>
#include <string.h>
#include <screen.h>

struct fs devfs = {
	0,0,
	{
		(fopen_t) dev_fopen,
		(fclose_t) dev_fclose,
		0,0,0,0,0,0,0
	},
	{
		0,
		(dopen_t) dev_dopen,
		(dclose_t) dev_dclose,
		(dclose_t) dev_dread
	}
};

size_t free_index = 0;
struct DEVICE_LIST {
	DEVICE *dev;
	struct DEVICE_LIST *next;
};
struct DEVICE_LIST devlist = {0, 0};

FILE *dev_fopen(struct fs *this, const char * filename, uint_t mode)
{
	struct DEVICE_LIST *list;

	list = &devlist;
	while (list) {
		if (list->dev && list->dev->name)
		if (strcmp(filename, list->dev->name) == 0) {
			return list->dev->devopen(list->dev, mode);
		}
		list = list->next;
	}
	return 0;
}

int dev_fclose(FILE *stream)
{
	struct DEVICE *dev = (struct DEVICE *)stream;

	/* Vapautus */

	kfree(dev);
	return 0;
}

int dev_dmake(struct fs *this, const char * dirname, uint_t owner, uint_t rights)
{
	return -1;
}

DIR *dev_dopen(struct fs *this, const char * dirname)
{
	return 0; /* TODO: Listaus */
}

int dev_dclose(DIR *listing)
{
	return -1;
}

int dev_dread(DIR *listing)
{
	return -1;
}

int devmanager_init(void)
{
	return 0;
}

int devmanager_uninit(void)
{
	struct DEVICE_LIST *list, *ptr;
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

int device_insert(struct DEVICE_ENTRY *device)
{
	struct DEVICE_LIST *list;

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
		list->next = kcalloc(1, sizeof(struct DEVICE_LIST));
	}
	list->dev = device;
	device->index = ++free_index;
	kprintf("Devmanager: Added device '%s' with index %d\n", device->name, device->index);
	return 0;
}
