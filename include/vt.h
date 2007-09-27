#ifndef _VT_H
#define _VT_H

#include <filesys/filesystem.h>
#include <spinlock.h>
#include <mem.h>
#include <io.h>
#include <irq.h>
#include <thread.h>
#include <putkaos.h>
#include <malloc.h>

#define VT_BUF_H 60 //pitää olla >= SCREEN_H
//#define VT_BUFFER_SIZE (VT_BUF_H*SCREEN_MEM_W)
#define VT_COUNT 6
#define VT_KERN_LOG 0 /* we put our log messages from kernel on that vt */

#define KB_BUFFER_SIZE 128

//referenssi-implementaatio display-ajurille löytyy display.c/h:sta

struct displayinfo {
	unsigned int w, h;
};

enum {
	IOCTL_DISPLAY_GET_INFO,
	IOCTL_DISPLAY_LOCATE,
	IOCTL_DISPLAY_CLS,
	IOCTL_DISPLAY_ROLL_UP
};

struct vt_t {
	volatile int kb_buf[KB_BUFFER_SIZE];
	volatile int kb_buf_start, kb_buf_end, kb_buf_count;

	char * buffer;
	unsigned int bufw, bufh; //buffer w, h (actual width is w*2)
	int bufsize;
	int scroll; /* how many lines have we scrolled up */
	unsigned int cx, cy; /* cursor x, y */
	unsigned char color;
	struct spinlock printlock; /* we are going to get problems with these spinlocks, we should replace them with something better later */
	struct spinlock writelock;
	unsigned char in_kprintf;
};

extern unsigned char vt_get_color(unsigned int vt_num);
extern void vt_set_color(unsigned int vt_num, unsigned char c);
extern int vt_locate(unsigned int vt_num, unsigned int x, unsigned int y);
extern void vt_cls(unsigned int vt_num);
//extern int vt_fastprint(unsigned int vt_num, const char *buf, unsigned int len);
extern int vt_print(unsigned int vt_num, const char *string);
extern int vt_putch(unsigned int vt_num, int c);
extern void vt_change(unsigned int vt_num);
extern void vt_scroll(int lines);
extern unsigned int vt_get_display_height(void);

extern unsigned int vt_out_get(void);
extern int vt_kb_get(unsigned int vt_num);
extern int vt_kb_peek(unsigned int vt_num);
extern void vt_keyboard(int code, int down);

extern void vt_kprintflock(unsigned int vt_num);
extern void vt_kprintfunlock(unsigned int vt_num);
extern void vt_unlockspinlocks(void);

extern void vt_init(void);
extern int vt_setdriver(char *filename);

//extern struct vt_t vt[VT_COUNT];
//extern unsigned int cur_vt;

#endif

