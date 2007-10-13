#include <filesys/fileutils.h>
#include <filesys/mount.h>

int link (const char *name, const char *linkname)
{
	const struct mount *mnt;

	// TODO: link: tänne vai muualle: link("/f", "/hak/") => link("/f", "/hak/f");
	// TODO: link: tänne vai muualle: link("/f/", ...) = error, ei linkitetä hakemistoja!

	if (name && *name && linkname && *linkname)
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

	if (name && *name && linkname && *linkname)
	if ((mnt = mount_etsi_kohta(&linkname)))
	if (mnt->fs && mnt->fs->fileutils.symlink) {
		return mnt->fs->fileutils.symlink(mnt->fs, name, linkname);
	}
	return -1;
}

int unlink (const char *name)
{
	const struct mount *mnt;

	if (name && *name)
	if ((mnt = mount_etsi_kohta(&name)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		return mnt->fs->fileutils.unlink(mnt->fs, name);
	}
	return -1;
}

int getprops (const char *name, struct file_props *val)
{
	const struct mount *mnt;

	if (name && *name && val)
	if ((mnt = mount_etsi_kohta(&name)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		return mnt->fs->fileutils.getprops(mnt->fs, name, val);
	}
	return -1;
}

int setprops (const char *name, const struct file_props *val)
{
	const struct mount *mnt;

	if (name && *name && val)
	if ((mnt = mount_etsi_kohta(&name)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		return mnt->fs->fileutils.setprops(mnt->fs, name, val);
	}
	return -1;
}
