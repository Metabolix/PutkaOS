#include <filesys/file.h>
#include <filesys/mount.h>
#include <string.h>
#include <stdint.h>

struct filefunc nil_filefunc = {0};

FILE *fopen(const char * filename, const char * mode)
{
	uint_t intmode = 0;
	if (!filename || !mode) {
		return 0;
	}
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
	return fopen_intflags(filename, intmode);
}

FILE *fopen_intflags(const char * filename, uint_t intmode)
{
	const struct mount *mnt;

	if (filename)
	if (intmode & (FILE_MODE_READ | FILE_MODE_WRITE))
	if (!(intmode & ~FILE_MODE_ALL))
	if ((mnt = mount_etsi_kohta(&filename)))
	if (mnt->fs)
	if (mnt->fs->filefunc.fopen) {
		FILE *f = mnt->fs->filefunc.fopen(mnt->fs, filename, intmode);
		if (!f) return f;
		if (!f->func) f->func = &nil_filefunc;
		if (f->mode == 0) f->mode = intmode;
		return f;
	}
	return 0;
}

int fclose(FILE *stream)
{
	if (stream && stream->func->fclose) {
		return stream->func->fclose(stream);
	}
	return -1;
}

size_t fread(void *buf, size_t size, size_t count, FILE *stream)
{
	if (!stream) return 0;
	if (size == 0 || count == 0) return 0;
	if (stream->func->fread) {
		return stream->func->fread(buf, size, count, stream);
	}
	return 0;
}

size_t fwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
	if (!stream) return 0;
	if (size == 0 || count == 0) return 0;
	if (stream->mode & FILE_MODE_APPEND) {
		if (fsetpos(stream, &stream->size)) {
			return 0;
		}
	}
	if (stream->func->fwrite) {
		return stream->func->fwrite(buf, size, count, stream);
	}
	return 0;
}

int fgetpos(FILE *stream, fpos_t *pos)
{
	if (stream) {
		*pos = stream->pos;
		return 0;
	}
	memset(pos, 0, sizeof(fpos_t));
	return EOF;
}

int fsetpos(FILE *stream, const fpos_t *pos)
{
	if (stream && stream->func->fsetpos) {
		return stream->func->fsetpos(stream, pos);
	}
	return EOF;
}

int fflush(FILE *stream)
{
	if (stream && stream->func->fflush) {
		return stream->func->fflush(stream);
	}
	return EOF;
}

int fseek(FILE *stream, long int offset, int origin)
{
	fpos_t pos;
	switch (origin) {
		case SEEK_SET:
			pos = offset;
			break;
		case SEEK_END:
			pos = stream->size - offset;
			break;
		case SEEK_CUR:
			pos = stream->pos + offset;
			break;
		default:
			return EOF;
	}
	return (fsetpos(stream, &pos) ? EOF : 0);
}

long ftell(FILE *stream)
{
	fpos_t pos;
	if (fgetpos(stream, &pos)) {
		return -1L;
	}
	return (long)(pos & INT32_MAX);
}
