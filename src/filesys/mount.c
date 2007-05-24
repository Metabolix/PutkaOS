#include <devices/devmanager.h>
#include <filesys/mount.h>
#include <filesys/filesystem.h>
#include <filesys/file.h>
#include <filesys/dir.h>
#include <malloc.h>
#include <screen.h>
#include <panic.h>
#include <string.h>

/*
* Internals
**/
static const char *parse_mboot_device_root(unsigned long mboot_device);
static const char *parse_mboot_cmdline_root(const char *str);
static struct mount *etsi_laite_rek(const char * device_name, struct mount *mnt);
static struct mount *etsi_kohta(const char ** filename_ptr);
static int umount_array(struct mount *array, size_t size);
static int umount(struct mount *mnt, struct mount *ret);

#define MAKE_NULL(b, a); {if (a) { b(a); (a) = 0; }}

char tyhja_string[1] = "";
struct mount root = {
	tyhja_string, "/", tyhja_string,
	0, 0,
	0,
	0, 0
};
size_t mount_count_ = 0;

/**
 * Etsii kohdan, johon laite on liitetty
 * (static)
**/
struct mount *etsi_laite_rek(const char * device_name, struct mount *mnt)
{
	int i;
	struct mount *retval;
	if (strcmp(mnt->dev_name, device_name) == 0) {
		return mnt;
	}
	i = mnt->subtree_size;
	while (i) {
		--i;
		retval = etsi_laite_rek(device_name, mnt->subtree + i);
		if (retval) {
			return retval;
		}
	}
	return 0;
}

/**
 * Etsii kohdan, jättää loput tiedostonimestä jäljelle.
 * f("/mnt/piste/joku/file"); => return mountpoint("/mnt/piste"), filename_ptr = "joku/file"
 * (static)
**/
struct mount *etsi_kohta(const char ** filename_ptr)
{
	int i, j;
	const char *filename = *filename_ptr;
	const char *newfilename;
	struct mount *mnt = &root;

	if (filename[0] == '/') {
		++filename;
		if (filename[0] == 0) {
			*filename_ptr = filename;
			return mnt;
		}
	} else {
		return 0;
	}
silmukka:
	for (i = 0; i < mnt->subtree_size; ++i) {
		newfilename = strrmsame(filename, mnt->subtree[i].relative_path);
		j = newfilename - filename;

		if (mnt->subtree[i].relative_path[j] == 0) {
			if (newfilename[0] == 0) {
				mnt = mnt->subtree + i;
				filename = newfilename;
				goto oikea_paikka;
			}
			if (newfilename[0] == '/') {
				mnt = mnt->subtree + i;
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

const char *parse_mboot_cmdline_root(const char *str)
{
	const char *pos, *end1, *end2;
	char *result;
	if (!str) {
		return 0;
	}
	pos = strstr(str, "root=");
	if (!pos || (pos != str && pos[-1] != ' ' && pos[-1] != '\t')) {
		kprintf("No root= given!!! ('%s')\n", str);
		return 0;
	}
	pos += 5;
	end1 = strchr(pos, ' ');
	end2 = strchr(pos, '\t');
	if (end2 && (end2 < end1)) {
		end1 = end2;
	}
	if (!end1) {
		for (end1 = pos; *end1; ++end1);
		if (end1 == pos) {
			return 0;
		}
	}
	result = kmalloc((end1 - pos) + 1 + 2); // tilaa myös /:lle, juuren "polulle"
	if (!result) {
		return 0;
	}
	memcpy(result, pos, (end1 - pos));
	result[(end1 - pos)] = 0;
	return result;
}
const char *parse_mboot_device_root(unsigned long mboot_device)
{
	/* TODO */
	return 0;
}
/**
 * Käynnistää mount-systeemin
**/
int mount_init(unsigned long mboot_device, const char *mboot_cmdline)
{
	uint_t flags;
	const char *root_device;
	static const char rootdev_default[] = "/dev/fd0";
	if (!(root_device = parse_mboot_cmdline_root(mboot_cmdline))) {
		if (!(root_device = parse_mboot_device_root(mboot_device))) {
			kprintf("mount: defaulting root to '%s'\n", rootdev_default);
			root_device = rootdev_default;
		}
	}

	/* Ensin /dev */
	root.subtree_size = 1;
	root.subtree = kcalloc(1, sizeof(struct mount));
	if (!root.subtree) {
		panic("mount: Out of kernel memory!\n");
	}

	root.subtree->dev_name = kmalloc(7 + 5); // "devman" + "/dev"
	root.subtree->absolute_path = root.subtree->dev_name + 7;
	root.subtree->relative_path = root.subtree->absolute_path + 1;
	root.subtree->subtree_size = 0;
	root.subtree->subtree = 0;
	root.subtree->dev = 0;
	root.subtree->fs = &devfs;
	strcpy(root.subtree->dev_name, "devman");
	strcpy(root.subtree->absolute_path, "/dev");
	mount_count_ = 1;

	/* Sitten / */
	flags = FILE_MODE_READ | FILE_MODE_WRITE;
	root.dev = fopen(root_device, "r+");
	if (!root.dev) {
		flags = FILE_MODE_READ;
		root.dev = fopen(root_device, "r");
	}
	if (!root.dev) {
		kprintf("mount: Couldn't open root device (%s).\n", root_device);
		/* TODO: Creating ramfs instead...\n */
		return -1;
	}

	root.fs = fs_mount(root.dev, flags);
	if (!root.fs) {
		kprintf("mount: Couldn't mount root device (%s).\n", root_device);
		return -1;
	}
	if (!root.fs->filefunc.fwrite) {
		kprintf("mount: Warning: root device (%s) mounted read-only!\n", root_device);
	}

	size_t dev_len = strlen(root_device);
	if (root_device == rootdev_default) {
		root.dev_name = kmalloc(dev_len + 3);
		memcpy(root.dev_name, root_device, dev_len + 1);
	} else {
		root.dev_name = (char*) root_device;
	}
	root.absolute_path = root.dev_name + dev_len + 1;
	root.relative_path = root.absolute_path + 1;
	root.absolute_path[0] = '/';
	root.absolute_path[1] = 0;

	mount_count_ = 2;
	return 0;
}

/**
 * Irrottaa taulukollisen rekursiivisesti
 * Käytetään vain mount_uninitissa!
 * (static)
**/
int umount_array(struct mount *array, size_t size)
{
	int i = 0;
	if (array) while (size) {
		--size;
		if (umount_array(array[size].subtree, array[size].subtree_size)) {
			i = -1;
		} else {
			MAKE_NULL(kfree, array[size].subtree);
			array[size].subtree_size = 0;

			if (umount(array + size, 0)) {
				i = -1;
			}
		}
	}
	return i;
}

/**
 * Lopettaa mount-systeemin
 * Jatkuu virheistä huolimatta! Ehkä jotain saa silloin pelastettua.
**/
void mount_uninit(void)
{
	int i;
	if ((i = umount_array(root.subtree, root.subtree_size)))
	if ((i = umount_array(root.subtree, root.subtree_size)))
	if ((i = umount_array(root.subtree, root.subtree_size))) {
		kprintf("mount: uninit: warning: not everything was umounted.\n");
		goto elsen_yli_1;
	}
	root.subtree_size = 0;
	MAKE_NULL(kfree, root.subtree);
	MAKE_NULL(kfree, root.dev_name);
elsen_yli_1:
	if ((i = root.fs->fs_umount(root.fs)))
	if ((i = root.fs->fs_umount(root.fs)))
	if ((i = root.fs->fs_umount(root.fs))) {
		kprintf("mount: uninit: (%s) => (%s): umount failed.\n", root.dev, root.absolute_path);
		goto elsen_yli_2;
	}
	root.fs = 0;
elsen_yli_2:
	if ((i = fclose(root.dev))) {
		kprintf("mount: uninit: (%s) == (%s): fclose failed. (WTF? :D)\n", root.dev, root.absolute_path);
	} else {
		root.dev = 0;
		MAKE_NULL(kfree, root.dev_name);
	}
}

/**
 * Liittää laitteen hakemistoon
**/
int mount_something(const char * device_filename, const char * mountpoint, uint_t flags)
{
	const char * point_rel;
	struct mount uusi = {
		0, 0, 0,
		0, 0,
		0, 0
	};
	struct mount *point_mnt;
	int mnt_len, rel_len, dev_len;
	int keno_lopussa;

	point_rel = mountpoint;
	point_mnt = etsi_kohta(&point_rel);

	if (!point_mnt) {
		kprintf("mount: (%s) => (%s): Point not found!\n", device_filename, mountpoint);
		return MOUNT_ERR_TOTAL_FAILURE;
	}

	rel_len = strlen(point_rel);
	keno_lopussa = ((point_rel[rel_len - 1] == '/') ? 1 : 0);
	mnt_len = rel_len + (point_rel - mountpoint) - keno_lopussa;
	dev_len = strlen(device_filename);

	if (!(rel_len - keno_lopussa)) {
		kprintf("mount: (%s) => (%s): Point in use!\n", device_filename, mountpoint);
		return MOUNT_ERR_ALREADY_MOUNTED;
	}

	if (!(uusi.dev = fopen_intflags(device_filename, flags))) {
		kprintf("mount: (%s) => (%s): Couldn't open device.\n", device_filename, mountpoint);
		return MOUNT_ERR_DEVICE_ERROR;
	}
	if (!(flags & FILE_MODE_WRITE)) {
		uusi.dev->func->fwrite = 0;
		// TODO: Flagit xD
	}

	uusi.fs = fs_mount(uusi.dev, flags);
	if (!uusi.fs) {
		kprintf("mount: (%s) => (%s): Couldn't mount device.\n", device_filename, mountpoint);
		MAKE_NULL(fclose, uusi.dev);
		return MOUNT_ERR_FILESYS_ERROR;
	}
	if (!uusi.fs->filefunc.fwrite && (flags & FILE_MODE_WRITE)) {
		kprintf("mount: (%s) => (%s): Warning: mounted read-only!\n", device_filename, mountpoint);
	}

	uusi.dev_name = kmalloc(mnt_len + 1 + dev_len + 1);
	uusi.absolute_path = uusi.dev_name + dev_len + 1;

	memcpy(uusi.dev_name, device_filename, dev_len + 1);
	memcpy(uusi.absolute_path, mountpoint, mnt_len + 1);

	uusi.relative_path = uusi.absolute_path + (point_rel - mountpoint);

	uusi.parent = point_mnt;
	++uusi.parent->subtree_size;
	uusi.parent->subtree = krealloc(uusi.parent->subtree, uusi.parent->subtree_size * sizeof(struct mount));
	uusi.parent->subtree[uusi.parent->subtree_size - 1] = uusi;

	return 0;
}

/**
 * Liittää uuden laitteen hakemistoon vanhan paikalle
**/
int mount_replace(const char * device_filename, const char * mountpoint, uint_t flags)
{
	const char * point_rel;
	const char * device_rel;
	struct mount uusi;
	struct mount *point_mnt, *device_mnt;
	int mnt_len, rel_len, dev_len;
	int keno_lopussa;

	point_rel = mountpoint;
	point_mnt = etsi_kohta(&point_rel);
	device_rel = device_filename;
	device_mnt = etsi_kohta(&device_rel);

	if (!point_mnt) {
		kprintf("mount_replace: (%s) => (%s): Point not found!\n", device_filename, mountpoint);
		return MOUNT_ERR_TOTAL_FAILURE;
	}
	if (!device_mnt || !device_mnt->fs->filefunc.fopen) {
		kprintf("mount_replace: (%s) => (%s): Device not found!\n", device_filename, mountpoint);
		return MOUNT_ERR_TOTAL_FAILURE;
	}

	uusi = *point_mnt;

	rel_len = strlen(point_rel);
	keno_lopussa = ((point_rel[rel_len - 1] == '/') ? 1 : 0);
	mnt_len = rel_len + (point_rel - mountpoint) - keno_lopussa;
	dev_len = strlen(device_filename);

	if (rel_len - keno_lopussa > 0) {
		kprintf("mount_replace: (%s) => (%s): Point not found!\n", device_filename, mountpoint);
		return MOUNT_ERR_TOTAL_FAILURE;
	}
	if (device_mnt == point_mnt) {
		kprintf("mount_replace: (%s) => (%s): Can't unmount old point, device is there!\n", device_filename, mountpoint);
		return MOUNT_ERR_TOTAL_FAILURE;
	}

	if (!(uusi.dev = fopen_intflags(device_filename, flags))) {
		kprintf("mount_replace: (%s) => (%s): Couldn't open device.\n", device_filename, mountpoint);
		return MOUNT_ERR_DEVICE_ERROR;
	}
	if (!(flags & FILE_MODE_WRITE)) {
		uusi.dev->func->fwrite = 0;
		// TODO: Flagit xD
	}

	uusi.fs = fs_mount(uusi.dev, flags);
	if (!uusi.fs) {
		kprintf("mount_replace: (%s) => (%s): Couldn't mount device.\n", device_filename, mountpoint);
		MAKE_NULL(fclose, uusi.dev);
		return MOUNT_ERR_FILESYS_ERROR;
	}
	if (!uusi.fs->filefunc.fwrite && (flags & FILE_MODE_WRITE)) {
		kprintf("mount_replace: (%s) => (%s): Warning: mounted read-only!\n", device_filename, mountpoint);
	}

	if (point_mnt->fs->fs_umount(point_mnt->fs)) {
		MAKE_NULL(uusi.fs->fs_umount, uusi.fs);
		MAKE_NULL(fclose, uusi.dev);
		kprintf("mount_replace: (%s) => (%s): Can't unmount old point!\n", device_filename, mountpoint);
		return MOUNT_ERR_FILESYS_ERROR;
	}
	if (fclose(point_mnt->dev)) {
		kprintf("mount_replace: (%s) => (%s): Warning: Old device not closed!\n", device_filename, mountpoint);
	}

	mnt_len = strlen(point_mnt->absolute_path);
	uusi.dev_name = kmalloc(dev_len + 1 + mnt_len + 1);
	uusi.absolute_path = uusi.dev_name + dev_len + 1;

	memcpy(uusi.dev_name, device_filename, dev_len + 1);
	memcpy(uusi.absolute_path, point_mnt->absolute_path, mnt_len + 1);

	uusi.relative_path = uusi.absolute_path + (point_mnt->relative_path - point_mnt->absolute_path);

	kfree(point_mnt->dev_name);
	*point_mnt = uusi;

	return 0;
}

/**
 * Irrottaa liitospisteen
**/
int umount(struct mount *mnt, struct mount *ret)
{
	int i;

	if (mnt->subtree_size) {
		if (ret) {
			ret->subtree = mnt->subtree;
			ret->subtree_size = mnt->subtree_size;
		} else {
			return MOUNT_ERR_MOUNTED_SUBPOINTS;
		}
	}
	switch (i = mnt->fs->fs_umount(mnt->fs)) {
		case MOUNT_ERR_BUSY:
			return MOUNT_ERR_BUSY;
		case 0:
			mnt->fs = 0;
			MAKE_NULL(fclose, mnt->dev);

			MAKE_NULL(kfree, mnt->dev_name);
			mnt->absolute_path = mnt->relative_path = 0;
			if (!ret) {
				mnt->subtree_size = 0;
				MAKE_NULL(kfree, mnt->subtree);
				if (mnt->parent) {
					--mnt->parent->subtree_size;
					*mnt = mnt->parent->subtree[mnt->parent->subtree_size];
				}
			}
			return 0;
		default:
			kprintf("umount: Failed at '%s' (%s)!\n", mnt->absolute_path, mnt->dev_name);
			kprintf("umount: ... Just told us %d, F*cking fs driver!\n", i);
			return MOUNT_ERR_TOTAL_FAILURE;
	}
#if 0
	switch (i = fclose(mnt->dev)) {
		case 0:
			return 0;
		default:
			kprintf("umount: fclose failed (WTF? :D) at device '%s' (%s)\n", mnt->dev_name, mnt->absolute_path);
	}
#endif
	return MOUNT_ERR_TOTAL_FAILURE;
}

/**
 * Irrottaa pisteen tai laitteen
**/
int umount_something(const char * device_or_point)
{
	struct mount *mnt;
	const char *filename;

	mnt = etsi_laite_rek(device_or_point, &root);
	if (mnt) {
		return umount(mnt, 0);
	}

	filename = device_or_point;
	mnt = etsi_kohta(&filename);
	if (mnt && filename[0] == 0) {
		return umount(mnt, 0);
	}
	kprintf("umount: %s isn't a point or a device\n", device_or_point);
	return MOUNT_ERR_TOTAL_FAILURE;
}

/**
 * Etsii tiedostonimen perusteella lähimmän liitospisteen ja muuttaa osoitinta niin, että siinä on tiedostonimen loppuosa
**/
const struct mount *mount_etsi_kohta(const char ** filename_ptr)
{
	return etsi_kohta(filename_ptr);
}

/**
 * Pisteiden lukumäärä
**/
int mount_count(void)
{
	return mount_count_;
}

static int mount_foreach_rek(mount_foreach_func_t f, struct mount *mnt, int level)
{
	struct mount *last;
	int ret = 1;
	f(mnt->fs->name, mnt->dev_name, mnt->absolute_path, mnt->relative_path, level);
	if (!mnt->subtree_size) {
		return ret;
	}
	last = mnt->subtree + mnt->subtree_size;
	for (mnt = mnt->subtree; mnt != last; ++mnt) {
		ret += mount_foreach_rek(f, mnt, level + 1);
	}
	return ret;
}

int mount_foreach(mount_foreach_func_t f)
{
	return mount_foreach_rek(f, &root, 0);
}
