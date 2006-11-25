#include <mem.h>

void *memcpy(void *dest, const void *src, unsigned int n)
{
	char * source = (char *)src;
	char * des = dest;

	for(; n; n--)
		*(des++) = *(source++);
	return dest;
}
	
void *memset(void *s, unsigned char c, unsigned int n)
{
	char * p = (char *) s;

	for(; n; n--)
		*(p++) = c;
	return s;
}

