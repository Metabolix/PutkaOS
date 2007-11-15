#include <stdio.h>
#include <sys/syscalls.h>
#include <sys/file.h>

void print(const char *str)
{
	syscall_print(str);
}

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
	return fopen2(filename, intmode);
}

int fseek(FILE *stream, long int offset, int origin)
{
	fseek_params_t params = {
		.f = stream,
		.offset = offset,
		.origin = origin,
	};

	switch (origin) {
		case SEEK_SET: {
			fpos_t pos = offset;
			return (fsetpos(stream, &pos) ? EOF : 0);
		}
		case SEEK_END: {
			return syscall_fseek(&params);
		}
		case SEEK_CUR: {
			return syscall_fseek(&params);
		}
	}
	return EOF;
}

long ftell(FILE *stream)
{
	fpos_t pos;
	if (fgetpos(stream, &pos)) {
		return -1L;
	}
	return (long)(pos & INT32_MAX);
}

int fgetpos(FILE *stream, fpos_t *pos)
{
	if (!stream || !pos) {
		return EOF;
	}
	return syscall_fgetpos(stream, pos);
}

int fsetpos(FILE *stream, const fpos_t *pos)
{
	if (!stream || !pos) {
		return EOF;
	}
	return syscall_fsetpos(stream, pos);
}

int fflush(FILE *stream)
{
	if (!stream) {
		return EOF;
	}
	return syscall_fflush(stream);
}

int fclose(FILE *stream)
{
	if (!stream) {
		return EOF;
	}
	return syscall_fclose(stream);
}

size_t fwrite(const void *buf, size_t size, size_t count, FILE *f)
{
	if (!buf || !size || !count || !f) {
		return 0;
	}
	fwrite_params_t params = {
		.buf = buf,
		.size = size,
		.count = count,
		.f = f,
	};
	return syscall_fwrite(&params);
}

size_t fread(void *buf, size_t size, size_t count, FILE *f)
{
	if (!buf || !size || !count || !f) {
		return 0;
	}
	fread_params_t params = {
		.buf = buf,
		.size = size,
		.count = count,
		.f = f,
	};
	return syscall_fread(&params);
}
