#include <filesys/file.h>
#include <filesys/mount.h>

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
	return mnt->fs->filefunc.fopen(mnt->fs, filename, intmode);
}

int fclose(FILE *file)
{
	if (!file || !file->func) {
		return -1;
	}
	return file->func->fclose(file);
}

size_t fread(void *buf, size_t size, size_t count, FILE *file)
{
	return 0;
}

size_t fwrite(const void *buf, size_t size, size_t count, FILE *file)
{
	return 0;
}

int fgetpos(FILE *stream, fpos_t *pos)
{
	return -1;
}

int fsetpos(FILE *stream, const fpos_t *pos)
{
	return -1;
}

int fseek(FILE *stream, long int offset, int origin)
{
	return -1;
}

long ftell(FILE *stream)
{
	return -1L;
}

int fflush(FILE *stream)
{
	return EOF;
}
