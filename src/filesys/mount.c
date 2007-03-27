#include <devmanager.h>
#include <filesys/mount.h>
#include <filesys/filesystem.h>
#include <filesys/file.h>
#include <filesys/dir.h>
#include <malloc.h>
#include <screen.h>
#include <panic.h>
#include <string.h>

const struct mountpoint *mount_etsi_kohta(const char ** filename_ptr)
{
	return etsi_kohta(filename_ptr);
}

#define MAKE_NULL(b, a); {if (a) { b(a); (a) = 0; }}

char tyhja_string[1] = "";
struct mountpoint root = {
	tyhja_string, "/", tyhja_string,
	0, 0,
	0, 0
};

/**
 * Etsii kohdan, johon laite on liitetty
**/
struct mountpoint *etsi_laite_rek(const char * device_name, struct mountpoint *mnt)
{
	int i;
	struct mountpoint *retval;
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
**/
struct mountpoint *etsi_kohta(const char ** filename_ptr)
{
	int i, j;
	const char *filename = *filename_ptr;
	const char *newfilename;
	struct mountpoint *mnt = &root;

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
	result = kmalloc((end1 - pos) + 1);
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
	root.subtree = kcalloc(1, sizeof(struct mountpoint));
	if (!root.subtree) {
		panic("mount: Out of kernel memory!\n");
	}

	root.subtree->dev_name = kmalloc(7);
	root.subtree->absolute_path = kmalloc(5);
	root.subtree->relative_path = root.subtree->absolute_path + 1;
	root.subtree->subtree_size = 0;
	root.subtree->subtree = 0;
	root.subtree->dev = 0;
	root.subtree->fs = &devfs;
	strcpy(root.subtree->absolute_path, "/dev");
	strcpy(root.subtree->dev_name, "devman");

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

	if (root_device != rootdev_default) {
		root.dev_name = kmalloc(strlen(root_device) + 1);
		memcpy(root.dev_name, root_device, strlen(root_device) + 1);
	} else {
		root.dev_name = (char*) root_device;
	}

	return 0;
}

/**
 * Irrottaa taulukollisen rekursiivisesti
 * Käytetään vain mount_uninitissa!
**/
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
		kprintf("mount: Uninit: warning: not everything was umounted.\n");
		goto elsen_yli_1;
	}
	root.subtree_size = 0;
	MAKE_NULL(kfree, root.subtree);
	MAKE_NULL(kfree, root.dev_name);
elsen_yli_1:
	if ((i = root.fs->fs_umount(root.fs)))
	if ((i = root.fs->fs_umount(root.fs)))
	if ((i = root.fs->fs_umount(root.fs))) {
		kprintf("mount: Uninit (%s) => (%s): umount failed.\n", root.dev, root.absolute_path);
		goto elsen_yli_2;
	}
	root.fs = 0;
elsen_yli_2:
	if ((i = fclose(root.dev))) {
		kprintf("mount: Uninit (%s) == (%s): fclose failed. (WTF? :D)\n", root.dev, root.absolute_path);
	} else {
		root.dev = 0;
	}
}

/**
 * Liittää laitteen hakemistoon
**/
int mount_something(const char * device_filename, const char * mountpoint, int flags)
{
	const char * relative = mountpoint;
	struct mountpoint uusi = {
		0, 0, 0,
		0, 0,
		0, 0
	};
	int i, j;

	uusi.parent = etsi_kohta(&relative);
	if (!uusi.parent) {
		kprintf("mount: (%s) => (%s): Point not found!\n", device_filename, mountpoint);
	}
	i = strlen(relative);
	if (!i || (i == 1 && relative[0] == '/')) {
		kprintf("mount: (%s) => (%s): Point in use!\n", device_filename, mountpoint);
		return MOUNT_ERR_ALREADY_MOUNTED;
	}

	if (!(uusi.dev = devfs.filefunc.fopen(&devfs, device_filename, flags)))
	if (!(uusi.dev = devfs.filefunc.fopen(&devfs, device_filename, flags))) {
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
		fclose(uusi.dev);
		return MOUNT_ERR_FILESYS_ERROR;
	}
	if (!uusi.fs->filefunc.fwrite && (flags & FILE_MODE_WRITE)) {
		kprintf("mount: (%s) => (%s): Warning: mounted read-only!\n", device_filename, mountpoint);
	}

	if (relative[i-1] == '/') {
		j = -1; --i;
	} else {
		j = 0;
	}
	i = strlen(mountpoint) + j;
	uusi.absolute_path = kmalloc(i + 1);
	memcpy(uusi.absolute_path, mountpoint, i);
	uusi.absolute_path[i] = 0;
	uusi.relative_path = uusi.absolute_path + (relative - mountpoint);

	i = strlen(device_filename);
	uusi.dev_name = kmalloc(i + 1);
	memcpy(uusi.dev_name, device_filename, i);
	uusi.dev_name[i] = 0;

	++uusi.parent->subtree_size;
	uusi.parent->subtree = krealloc(uusi.parent->subtree, uusi.parent->subtree_size * sizeof(struct mountpoint));
	uusi.parent->subtree[uusi.parent->subtree_size - 1] = uusi;
	return 0;
}

/**
 * Irrottaa liitospisteen
**/
int umount_point(struct mountpoint *mnt)
{
	int i;

	if (mnt->subtree_size) {
		return MOUNT_ERR_MOUNTED_SUBPOINTS;
	}
	switch (i = mnt->fs->fs_umount(mnt->fs)) {
		case MOUNT_ERR_BUSY:
			return MOUNT_ERR_BUSY;
		case 0:
			MAKE_NULL(kfree, mnt->dev_name);
			MAKE_NULL(kfree, mnt->absolute_path);
			mnt->relative_path = 0;
			mnt->subtree_size = 0;
			MAKE_NULL(kfree, mnt->subtree);
			MAKE_NULL(fclose, mnt->dev);
			mnt->fs = 0;
			if (mnt->parent) {
				--mnt->parent->subtree_size;
				*mnt = mnt->parent->subtree[mnt->parent->subtree_size];
				// realloc? Mitäpä sillä, turhaa ajantuhlausta...
			}
			return 0;
		default:
			kprintf("mount: Umounting '%s' (%s) failed!\n", mnt->absolute_path, mnt->dev_name);
			kprintf("mount: ... Just told us %d, F*cking fs driver!\n", i);
			return MOUNT_ERR_TOTAL_FAILURE;
	}
	switch (i = fclose(mnt->dev)) {
		case 0:
			return 0;
		default:
			kprintf("mount: Umount: fclose failed (WTF? :D) at device '%s' (%s)\n", mnt->dev_name, mnt->absolute_path);
			return i;
	}
	return MOUNT_ERR_TOTAL_FAILURE;
}

/**
 * Irrottaa pisteen tai laitteen
**/
int umount_something(const char * device_or_point)
{
	struct mountpoint *mnt;
	const char *filename;

	mnt = etsi_laite_rek(filename, &root);
	if (mnt) {
		return umount_point(mnt);
	}

	filename = device_or_point;
	mnt = etsi_kohta(&filename);
	if (mnt && filename[0] == 0) {
		return umount_point(mnt);
	}
	kprintf("mount: Umount: %s isn't a point or a device\n", device_or_point);
	return MOUNT_ERR_TOTAL_FAILURE;
}
