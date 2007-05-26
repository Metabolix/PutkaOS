#ifndef _STRING_H
#define _STRING_H

#include <mem.h>

extern char    *strcat(char *, const char *);
extern char    *strchr(const char *, int);
extern int      strcmp(const char *, const char *);
//extern int      strcoll(const char *, const char *);
extern char    *strcpy(char *, const char *);
extern size_t   strcspn(const char *, const char *);
extern char    *strdup(const char *);
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
