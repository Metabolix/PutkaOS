#include <stdio.h>
#include <mount.h>

FILE *fopen(const char * filename, const char * mode)
{
	struct mountpoint *mnt;
	uint_t intmode = 0;
	if (!filename || !mode) {
		return 0;
	}
	mnt = etsi_kohta(&filename);
	/* parsitaan mode */
	for (intmode = 0; *mode; ++mode) {
		if (*mode == 'r') {
			intmode |= FILE_MODE_READ;
		} else if (*mode == 'w') {
			intmode |= FILE_MODE_WRITE | FILE_MODE_CLEAR;
		} else if (*mode == 'a') {
			intmode |= FILE_MODE_WRITE | FILE_MODE_APPEND;
		} else if (*mode == '+') {
			intmode |= FILE_MODE_WRITE | FILE_MODE_READ;
		}
	}
	return mnt->fs->fopen(mnt->fs, filename, intmode);
}

int fclose(FILE *file)
{
	if (!file || !file->fs) {
		return -1;
	}
	return file->fs->fclose(file);
}
