#include <filesys/fileutils.h>
#include <filesys/mount.h>

int link (const char *name, const char *linkname)
{
	const struct mount *mnt;

	if (name && linkname)
	if ((mnt = mount_etsi_kohta(&linkname)))
	if (mnt->fs && mnt->fs->fileutils.link)
	if (mnt == mount_etsi_kohta(&name)) {
		return mnt->fs->fileutils.link(mnt->fs, name, linkname);
	}
	return -1;
}

int symlink (const char *name, const char *linkname)
{
	const struct mount *mnt;

	if (name && linkname)
	if ((mnt = mount_etsi_kohta(&linkname)))
	if (mnt->fs && mnt->fs->fileutils.symlink) {
		return mnt->fs->fileutils.symlink(mnt->fs, name, linkname);
	}
	return -1;
}

int unlink (const char *name)
{
	const struct mount *mnt;

	if (name)
	if ((mnt = mount_etsi_kohta(&name)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		mnt->fs->fileutils.unlink(mnt->fs, name);
	}
	return -1;
}

int getprops (const char *name, struct file_props *val)
{
	const struct mount *mnt;

	if (name && val)
	if ((mnt = mount_etsi_kohta(&name)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		mnt->fs->fileutils.getprops(mnt->fs, name, val);
	}
	return -1;
}

int setprops (const char *name, const struct file_props *val)
{
	const struct mount *mnt;

	if (name && val)
	if ((mnt = mount_etsi_kohta(&name)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		mnt->fs->fileutils.setprops(mnt->fs, name, val);
	}
	return -1;
}
