#include <mem.h>
#include <stddef.h>

void *memchr(const void *s1, int c, size_t n)
{
	const unsigned char * s = s1;
	for(; n; --n, ++s) {
		if (*s == (unsigned char)c) {
			return (void*)s;
		}
	}
	return 0;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char * ss1 = (const unsigned char *)s1;
	const unsigned char * ss2 = (const unsigned char *)s2;

	for(; n; n--) {
		if (*ss1 != *ss2) {
			return (int)*ss1 - (int)*ss2;
		}
		++ss1;
		++ss2;
	}
	return 0;
}

void *memccpy(void *dest, const void *src, int c, size_t n)
{
	const unsigned char * source = (const unsigned char *)src;
	unsigned char * des = dest;

	for(; n; n--) {
		*des = *source;
		if (*des == (unsigned char)c) {
			return des + 1;
		}
	}
	return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	const unsigned char * source = (const unsigned char *)src;
	unsigned char * des = dest;

	for(; n; n--) {
		*(des++) = *(source++);
	}
	return dest;
}

void *memmove(void *dest0, const void *src0, size_t n)
{
	if (src0 == dest0) {
		return dest0;
	}
	const unsigned char * src = (const unsigned char *)src0;
	unsigned char * dest = dest0;
	if (src > dest) {
		for(; n; n--) {
			*(dest++) = *(src++);
		}
		return dest0;
	}

	dest += n;
	src += n;
	for(; n; n--) {
		*(--dest) = *(--src);
	}
	return dest0;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char * p = (unsigned char *) s;

	for(; n; n--)
		*(p++) = (unsigned char)c;
	return s;
}

