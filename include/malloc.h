#ifndef _MALLOC_H
#define _MALLOC_H
#include <stddef.h>
extern void malloc_init();
#define malloc(a) kmalloc(a)
extern void * kmalloc(size_t size);
extern void * kcalloc(size_t nmemb, size_t size);
extern void kfree(void * pointer);
#endif
