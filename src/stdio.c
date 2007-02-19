#include <stdio.h>

FILE *fopen(const char * restrict filename, const char * restrict mode)
{
	struct mount *mnt;
	mnt = etsi_kohta(&filename);
	return mnt->fs->fopen(mnt->fs, filename, mode);
}

int fclose(FILE *file)
{
	return file->fs->fclose(file);
}

