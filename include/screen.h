#ifndef _SCREEN_H
#define _SCREEN_H

#include <thread.h>
#include <stddef.h>
#include <malloc.h>
#include <spinlock.h>
#include <keyboard.h>

#define SCREEN_SIZE (80*25*2)
#define SCREEN_BUFFER_SIZE (SCREEN_SIZE*2)
#define VT_COUNT 6
#define VT_KERN_LOG 0 /* we put our log messages from kernel on that vt */

struct vt_t {
	char * buffer;
	volatile int kb_buff[kb_buffer_size];
	volatile int kb_buff_filled;
	int scroll;     /* how many lines have we scrolled */
	unsigned int cx, cy; /* cursor x, y */
	unsigned char colour;
	struct spinlock printlock; /* we are going to get problems with these spinlocks, we should replace them with something better later */
	struct spinlock writelock;
	unsigned char in_kprintf;
};

extern void cls(void);
extern int vt_out_get(void);
extern int print(const char * string);
extern void move_cursor(void);
extern void putch_vt(int c, int vt);
extern void putch(int c);
extern int move(unsigned int y, unsigned int x);
extern unsigned char get_color(void);
extern void set_color(unsigned char c);
extern void change_vt(unsigned int vt_n);
extern void vts_init(void);
extern void screen_init(void);
/*extern void scroll(int lines);*/

extern int kprintf(const char *fmt, ...);

extern struct vt_t vt[VT_COUNT];
extern unsigned int cur_vt;

#endif

