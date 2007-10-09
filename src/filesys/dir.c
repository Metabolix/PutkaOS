#include <filesys/dir.h>
#include <filesys/mount.h>

int dmake(const char * dirname)
{
	const struct mount *mnt;
	const char *newdirname = dirname;
	mnt = mount_etsi_kohta(&newdirname);
	if (!mnt || !mnt->fs || !(mnt->fs->mode & FILE_MODE_WRITE)) {
		return DIR_ERR_CANT_MAKE;
	}
	if (!mnt->fs->dirfunc.dopen) {
		return DIR_ERR_NO_FUNCTIONS;
	}
	return mnt->fs->dirfunc.dmake(mnt->fs, newdirname);
}

DIR *dopen(const char * dirname)
{
	const struct mount *mnt;
	const char *newdirname = dirname;
	mnt = mount_etsi_kohta(&newdirname);
	if (!mnt || !mnt->fs || !mnt->fs->dirfunc.dopen) {
		return 0;
	}
	return mnt->fs->dirfunc.dopen(mnt->fs, newdirname);
}

int dread(DIR *listing)
{
	if (!listing || !listing->func || !listing->func->dread) {
		return EOF;
	}
	return listing->func->dread(listing);
}

int dclose(DIR *listing)
{
	if (!listing || !listing->func || !listing->func->dclose) {
		return EOF;
	}
	return listing->func->dclose(listing);
}
