#ifndef _MALLOC_H
#define _MALLOC_H
#include <stddef.h>
//#define malloc(a) kmalloc(a)

extern void * kmalloc(size_t size);
extern void * malloc(size_t size);

extern void * kcalloc(size_t nmemb, size_t size);
extern void * calloc(size_t nmemb, size_t size);

extern void * krealloc(void *ptr, size_t size);
extern void * realloc(void *ptr, size_t size);

extern void kfree(void * pointer);
extern void free(void * pointer);

extern void malloc_init(void);
#define	MAX_ALLOCATIONS		512
#endif
