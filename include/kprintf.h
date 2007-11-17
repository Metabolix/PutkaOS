#ifndef _KPRINTF_H
#define _KPRINTF_H 1

#include <stdio.h>
#define kprintf(...) fprintf(stderr, __VA_ARGS__)
//extern int kprintf(const char *fmt, ...);

#endif
