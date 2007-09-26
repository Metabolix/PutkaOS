#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <thread.h>
#include <stddef.h>
#include <malloc.h>
#include <spinlock.h>
#include <keyboard.h>
#include <vt.h>

#define DISPLAY_W 80
#define DISPLAY_H 25
#define DISPLAY_MEM_W (DISPLAY_W*2)
#define DISPLAY_MEM_SIZE (DISPLAY_MEM_W*DISPLAY_H)

extern void display_init(void);

//extern int kprintf(const char *fmt, ...);

#endif

