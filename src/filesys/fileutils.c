#include <filesys/fileutils.h>
#include <filesys/mount.h>

int link (const char *src, const char *dest)
{
	const struct mount *mnt;

	if (src && dest)
	if ((mnt = mount_etsi_kohta(&dest)))
	if (mnt->fs && mnt->fs->fileutils.link)
	if (mnt == mount_etsi_kohta(&src)) {
		return mnt->fs->fileutils.link(mnt->fs, src, dest);
	}
	return -1;
}

int symlink (const char *src, const char *dest)
{
	const struct mount *mnt;

	if (src && dest)
	if ((mnt = mount_etsi_kohta(&dest)))
	if (mnt->fs && mnt->fs->fileutils.symlink) {
		return mnt->fs->fileutils.link(mnt->fs, src, dest);
	}
	return -1;
}

int unlink (const char *src)
{
	const struct mount *mnt;

	if (src)
	if ((mnt = mount_etsi_kohta(&src)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		mnt->fs->fileutils.unlink(mnt->fs, src);
	}
	return -1;
}

int getprops (const char *src, struct file_props *val)
{
	const struct mount *mnt;

	if (src && val)
	if ((mnt = mount_etsi_kohta(&src)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		mnt->fs->fileutils.getprops(mnt->fs, src, val);
	}
	return -1;
}

int setprops (const char *src, const struct file_props *val)
{
	const struct mount *mnt;

	if (src && val)
	if ((mnt = mount_etsi_kohta(&src)))
	if (mnt->fs && mnt->fs->fileutils.link) {
		mnt->fs->fileutils.setprops(mnt->fs, src, val);
	}
	return -1;
}
