#include <string.h>
#include <memory/malloc.h>
#include <stddef.h>

/*************
**  MEM *   **
*************/

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
/*
void *memset(void *s, int c, size_t n)
{
	if (n <= 0) return s;

	unsigned char * p = (unsigned char *) s + n;

	while (p != s) {
		*(--p) = c;
	}

	return s;
}
*/
/*************
**  STR *   **
*************/

char *strcat(char *s1, const char *s2)
{
	char *retval = s1;
	while (*s1) {
		++s1;
	}
	while (*s2) {
		*s1 = *s2;
		++s1; ++s2;
	}
	*s1 = 0;
	return retval;
}

char *strchr(const char *s, int c)
{
	while (*s && (unsigned char)*s != (unsigned char)c) {
		++s;
	}
	if ((unsigned char)*s != (unsigned char)c) {
		return 0;
	}
	return (char *)s;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2) {
		++s1; ++s2;
	}
	return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

/*
int strcoll(const char *s1, const char *s2)
{
	print("strcoll -> strcmp\n");
	return strcmp(s1, s2);
}
*/

char *strcpy(char *dest, const char *src)
{
	char *retval = dest;
	while (*src) {
		*dest = *src;
		++dest; ++src;
	}
	*dest = 0;
	return retval;
}

#if 0
size_t strcspn(const char *s1, const char *s2)
{
	char *buf = kmalloc(256);
	size_t retval = 0, current = 0;
	while (*s2) {
		buf[(unsigned char)*s2] = 1;
		++s2;
	}
	while (*s1) {
		if (buf[(unsigned char)*s1]) {
			if (current > retval) {
				retval = current;
			}
			current = 0;
		} else {
			++current;
		}
	}
	return retval;
}

char *strdup(const char *s)
{
	size_t len = strlen(s);
	char *retval = kmalloc(len + 1);
	if (!retval) {
		return 0;
	}
	memcpy(retval, s, len + 1);
	return retval;
}
#endif

size_t strlen(const char *s)
{
	size_t r = 0;
	while (*s) {
		++r; ++s;
	}
	return r;
}

char *strncat(char *s1, const char *s2, size_t n)
{
	char *retval = s1;
	while (*s1) {
		++s1;
	}
	while (*s2 && n) {
		*s1 = *s2;
		++s1; ++s2; --n;
	}
	*s1 = 0;
	return retval;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	if (!n) {
		return 0;
	}
	--n;
	while (*s1 && *s1 == *s2 && n) {
		++s1; ++s2; --n;
	}
	return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *retval = dest;
	while (*src && n) {
		*dest = *src;
		++dest; ++src; --n;
	}
	while (n) {
		*dest = 0;
		++dest; --n;
	}
	return retval;
}

char *strstr(const char *str, const char *seek)
{
	size_t i;
	while (*str) {
		for (i = 0; seek[i]; ++i) {
			if (seek[i] != str[i]) {
				break;
			}
		}
		if (!seek[i]) {
			return (char*)str;
		}
		++str;
	}
	return 0;
}

/*
char    *strpbrk(const char *, const char *);
char    *strrchr(const char *, int);
size_t   strspn(const char *, const char *);
char    *strstr(const char *, const char *);
char    *strtok(char *, const char *);
char    *strtok_r(char *, const char *, char **);
size_t   strxfrm(char *, const char *, size_t);
*/

char *strrmsame(const char *s1, const char *s2)
{
	size_t retval = 0;
	while (*s1 && *s1 == *s2) {
		++s1; ++s2; ++retval;
	}
	return (char *)s1;
}
