#include <string.h>
#include <mem.h>
//#include <malloc.h>

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
	return retval;
}

size_t strcspn(const char *s1, const char *s2)
{
	char buf[256] = {0};
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

/*
char *strdup(const char *s)
{
	size_t len = strlen(s);
	char *retval = malloc(len + 1);
	if (!retval) {
		return 0;
	}
	memcpy(retval, s, len + 1);
	return s;
}
*/

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
	return retval;
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
