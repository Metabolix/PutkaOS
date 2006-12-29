#ifndef _MALLOC_H
#define _MALLOC_H
#include <stddef.h>
extern void malloc_init();
extern void * kmalloc(size_t size);
extern void kfree(void * pointer);
#endif
