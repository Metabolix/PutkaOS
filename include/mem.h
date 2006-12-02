#ifndef _MEM_H
#define _MEM_H

#include <stddef.h>

extern void    *memccpy(void *dest, const void *src, int c, size_t n);
extern void    *memchr(const void *s, int c, size_t n);
extern int      memcmp(const void *s1, const void *s2, size_t n);
extern void    *memcpy(void *dest, const void *src, size_t n);
extern void    *memmove(void *dest, const void *src, size_t n);
extern void    *memset(void *s, int c, size_t n);

#endif
