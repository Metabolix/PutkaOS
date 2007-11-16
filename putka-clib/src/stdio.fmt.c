#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

struct fprintf_putter {
	putstr_t putstr;
	FILE *file;
};

struct sprintf_putter {
	putstr_t putstr;
	char *buf, *buf_end;
};

size_t fprintf_putstr(const char *str, size_t len, struct fprintf_putter *f)
{
	return fwrite(str, 1, len, f->file);
}

size_t sprintf_putstr(const char *str, size_t len, struct sprintf_putter *f)
{
	if (len > f->buf_end - f->buf) {
		len = f->buf_end - f->buf;
	}
	memcpy(f->buf, str, len);
	f->buf += len;
	return len;
}

size_t printf_putstrlen(const char *str, size_t len)
{
	char buf[len+1];
	memcpy(buf, str, len);
	buf[len] = 0;
	return print(buf);
}

size_t printf_putstr(const char *str, size_t len, putstr_t putter)
{
	if (!str[len]) {
		return print(str);
	}
	return printf_putstrlen(str, len);
}

int vsprintf(char * restrict buf, const char * restrict fmt, va_list args)
{
	struct sprintf_putter putter = {
		.putstr = (putstr_t) sprintf_putstr,
		.buf = buf,
		.buf_end = buf + INT32_MAX,
	};

	int ret = _xprintf(&putter.putstr, fmt, args);
	*putter.buf = 0;
	return ret;
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

int vfprintf(FILE * restrict f, const char * restrict fmt, va_list args)
{
	struct fprintf_putter putter = {
		.putstr = (putstr_t) fprintf_putstr,
		.file = f,
	};

	return _xprintf(&putter.putstr, fmt, args);
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

int vprintf(const char * restrict fmt, va_list args)
{
	putstr_t p = (putstr_t) printf_putstr;
	return _xprintf(&p, fmt, args);
}

int printf(const char * restrict fmt, ...)
{
	int retval;
	va_list args;
	va_start(args, fmt);
	retval = vprintf(fmt, args);
	va_end(args);
	return retval;
}
