#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

/*************
**  MEM *   **
*************/

extern void    *memccpy(void *dest, const void *src, int c, size_t n);
extern void    *memchr(const void *s, int c, size_t n);
extern int      memcmp(const void *s1, const void *s2, size_t n);
extern void    *memcpy(void *dest, const void *src, size_t n);
extern void    *memmove(void *dest, const void *src, size_t n);
extern void    *memset(void *s, int c, size_t n);

/*************
**  STR *   **
*************/

extern char    *strcat(char *, const char *);
extern char    *strchr(const char *, int);
extern int      strcmp(const char *, const char *);
// extern int      strcoll(const char *, const char *);
extern char    *strcpy(char *, const char *);
// extern size_t   strcspn(const char *, const char *);
// extern char    *strdup(const char *);
// extern char    *strerror(int);
extern size_t   strlen(const char *);
extern char    *strncat(char *, const char *, size_t);
extern int      strncmp(const char *, const char *, size_t);
extern char    *strncpy(char *, const char *, size_t);

// extern char    *strpbrk(const char *, const char *);
// extern char    *strrchr(const char *, int);
// extern size_t   strspn(const char *, const char *);

extern char    *strstr(const char *, const char *);

// extern char    *strtok(char *, const char *);
// extern size_t   strxfrm(char *, const char *, size_t);

/*
 * Non-standard
 */
extern char *strrmsame(const char *s1, const char *s2);

#endif
