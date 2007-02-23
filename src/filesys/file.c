#include <filesys/file.h>
#include <filesys/mount.h>
#include <string.h>

FILE *fopen(const char * filename, const char * mode)
{
	struct mountpoint *mnt;
	uint_t intmode = 0;
	if (!filename || !mode) {
		return 0;
	}
	mnt = etsi_kohta(&filename);

	if (!mnt || !mnt->fs || !mnt->fs->filefunc.fopen) {
		return 0;
	}
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

int fclose(FILE *stream)
{
	if (stream && stream->func && stream->func->fclose) {
		return stream->func->fclose(stream);
	}
	return -1;
}

size_t fread(void *buf, size_t size, size_t count, FILE *stream)
{
	if (stream && stream->func && stream->func->fread) {
		return stream->func->fread(buf, size, count, stream);
	}
	return 0;
}

size_t fwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
	if (stream && stream->func && stream->func->fwrite) {
		return stream->func->fwrite(buf, size, count, stream);
	}
	return 0;
}

int fgetpos(FILE *stream, fpos_t *pos)
{
	if (stream && stream->func && stream->func->fgetpos) {
		return stream->func->fgetpos(stream, pos);
	}
	memset(pos, 0, sizeof(fpos_t));
	return EOF;
}

int fsetpos(FILE *stream, const fpos_t *pos)
{
	if (stream && stream->func && stream->func->fsetpos) {
		return stream->func->fsetpos(stream, pos);
	}
	return EOF;
}

int fseek(FILE *stream, long int offset, int origin)
{
	if (stream && stream->func && stream->func->fseek) {
		return stream->func->fseek(stream, offset, origin);
	}
	return EOF;
}

long ftell(FILE *stream)
{
	if (stream && stream->func && stream->func->ftell) {
		return stream->func->ftell(stream);
	}
	return -1L;
}

int fflush(FILE *stream)
{
	if (stream && stream->func && stream->func->fflush) {
		return stream->func->fflush(stream);
	}
	return EOF;
}
