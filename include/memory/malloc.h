#ifndef _MALLOC_H
#define _MALLOC_H 1

#include <stddef.h>

extern void * malloc(size_t size);
extern void * calloc(size_t nmemb, size_t size);
extern void * realloc(void *ptr, size_t size);
extern void free(void * pointer);

#endif
