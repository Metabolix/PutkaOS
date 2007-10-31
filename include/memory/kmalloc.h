#ifndef _KMALLOC_H
#define _KMALLOC_H 1

#include <stddef.h>

extern void * kmalloc(size_t size);
extern void * kcalloc(size_t nmemb, size_t size);
extern void * krealloc(void *ptr, size_t size);
extern void kfree(void * pointer);

#endif
