#include <filesys/dir.h>
#include <filesys/mount.h>

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

int dmake(const char * dirname, uint_t owner, uint_t rights)
{
	const struct mount *mnt;
	const char *newdirname = dirname;
	mnt = mount_etsi_kohta(&newdirname);
	if (!mnt || !mnt->fs || !mnt->fs->dirfunc.dopen) {
		return 0;
	}
	return mnt->fs->dirfunc.dmake(mnt->fs, newdirname, owner, rights);
}

int dread(DIR *listing)
{
	if (!listing || !listing->func || !listing->func->dread) {
		return 0;
	}
	return listing->func->dread(listing);
}

int dclose(DIR *listing)
{
	if (!listing || !listing->func || !listing->func->dclose) {
		return 0;
	}
	return listing->func->dclose(listing);
}
