#ifndef _MALLOC_H
#define _MALLOC_H
#include <stddef.h>
extern void malloc_init(void);
#define malloc(a) kmalloc(a)
extern void * kmalloc(size_t size);
extern void * kcalloc(size_t nmemb, size_t size);
extern void * krealloc(void *ptr, size_t size);
extern void kfree(void * pointer);
#endif
