#include <stdio.h>

int fprintf(FILE * restrict f, const char * restrict fmt, ...)
{
	int retval;
	va_list args;
	va_start(args, fmt);
	retval = vfprintf(f, fmt, args);
	va_end(args);
	return retval;
}
