#ifndef _VT_H
#define _VT_H

#include <stddef.h>
#include <spinlock.h>
#include <keyboard.h>
#include <filesys/file.h>

enum {
	IOCTL_VT_SET_FWRITE
};

extern int vt_get(FILE **streams);
//extern FILE * vt_getpair(FILE *stream);

extern void vt_init(void);

#endif

