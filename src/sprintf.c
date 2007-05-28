#include <filesys/file.h>
#include <filesys/filesystem.h>
#include <screen.h>
#include <malloc.h>

struct sprintf_file {
	FILE std;
	char *buf;
	void *ret;
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

struct filefunc strfilefunc = {
	0, 0, 0, (fwrite_t) sprintf_fwrite, 0, 0, 0
};
FILE strfile = {
	0, 0, 0, 0, &strfilefunc
};

struct sprintf_file *sprintf_start(char *buf, void *retpos)
{
	struct sprintf_file *retval = kmalloc(sizeof(struct sprintf_file));
	retval->std = strfile;
	retval->buf = buf;
	retval->ret = retpos;
	return retval;
}

void sprintf_quit(struct sprintf_file *f, void **retpos)
{
	*f->buf = 0;
	*retpos = f->ret;
	kfree(f);
}
/*
int sprintf(char * restrict str, const char * restrict fmt, ...)
{
	struct sprintf_file f = {strfile, str};
	return fprintf(&f, fmt
}
*/
