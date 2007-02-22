#include <filesys/dir.h>
#include <filesys/mount.h>

DIR *dopen(const char * dirname)
{
	const struct mountpoint *mnt;
	const char *newdirname = dirname;
	mnt = mount_etsi_kohta(&newdirname);
	return mnt->fs->dirfunc.dopen(mnt->fs, newdirname);
}

int dmake(const char * dirname, uint_t owner, uint_t rights)
{
	const struct mountpoint *mnt;
	const char *newdirname = dirname;
	mnt = mount_etsi_kohta(&newdirname);
	return mnt->fs->dirfunc.dmake(mnt->fs, newdirname, owner, rights);
}

int dread(DIR *listing)
{
	return listing->func->dread(listing);
}

int dclose(DIR *listing)
{
	return listing->func->dclose(listing);
}
