#include <stdio.h>
#include <filesys/file.h>
#include <filesys/filesystem.h>
#include <screen.h>

struct sprintf_file {
	FILE std;
	char *buf;
};

size_t sprintf_fwrite(const char *buf, size_t size, size_t count, struct sprintf_file *f)
{
	size_t i, j;
	i = j = size * count;
	while (j--) {
		*f->buf = *buf;
		++f->buf;
		++buf;
		f->std.pos++;
		f->std.size++;
	}
	return i;
}

const struct filefunc strfilefunc = {
	.fwrite = (fwrite_t) sprintf_fwrite,
	//0, 0, 0, (fwrite_t) sprintf_fwrite, 0, 0, 0
};

int vsprintf(char * restrict buf, const char * restrict fmt, va_list args)
{
	struct sprintf_file f = {
		.std.func = &strfilefunc,
		.buf = buf,
	};

	return vfprintf(&f.std, fmt, args);
}

int sprintf(char * restrict buf, const char * restrict fmt, ...)
{
	int r;

	va_list args;
	va_start(args, fmt);
	r = vsprintf(buf, fmt, args);
	va_end(args);

	return r;
}

int fprintf(FILE * restrict f, const char * restrict fmt, ...)
{
	int retval;
	va_list args;
	va_start(args, fmt);
	retval = vfprintf(f, fmt, args);
	va_end(args);
	return retval;
}
