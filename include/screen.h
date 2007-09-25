#ifndef _SCREEN_H
#define _SCREEN_H

#include <thread.h>
#include <stddef.h>
#include <malloc.h>
#include <spinlock.h>
#include <keyboard.h>

#define SCREEN_W 80
#define SCREEN_H 25
//#undef SCREEN_BUFFER
#define SCREEN_BUFFER
#define SCREEN_MEM_W (SCREEN_W*2)
//#define SCREEN_BUF_H SCREEN_H
#define SCREEN_MEM_SIZE (SCREEN_MEM_W*SCREEN_H)
#define SCREEN_BUF_H 60
#define SCREEN_BUFFER_SIZE (SCREEN_BUF_H*SCREEN_MEM_W)
#define VT_COUNT 6
#define VT_KERN_LOG 0 /* we put our log messages from kernel on that vt */

struct vt_t {
	volatile int kb_buf[KB_BUFFER_SIZE];
	volatile int kb_buf_start, kb_buf_end, kb_buf_count;

	char * buffer;
	int scroll; /* how many lines have we scrolled up */
	unsigned int cx, cy; /* cursor x, y */
	unsigned char colour;
	struct spinlock printlock; /* we are going to get problems with these spinlocks, we should replace them with something better later */
	struct spinlock writelock;
	unsigned char in_kprintf;
};

extern void cls(void);
extern unsigned int vt_out_get(void);
extern int print(const char * string);
extern void move_cursor(void);
extern void putch_vt(int c, unsigned int vt);
#define putch(c) putch_vt(c, vt_out_get())
extern int move(unsigned int y, unsigned int x);
extern unsigned char get_colour(void);
extern void set_colour(unsigned char c);
extern void change_vt(unsigned int vt_n);
extern void vts_init(void);
extern void screen_init(void);
extern void scroll(int lines);

extern int kprintf(const char *fmt, ...);

extern struct vt_t vt[VT_COUNT];
extern unsigned int cur_vt;

#endif

