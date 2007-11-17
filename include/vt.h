#ifndef _VT_H
#define _VT_H

#include <filesys/filesystem.h>
#include <spinlock.h>
#include <string.h>
#include <io.h>
#include <irq.h>
#include <multitasking/multitasking.h>
#include <memory/kmalloc.h>

#define VT_BUF_H 60
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

enum { VT_MODE_NORMAL, VT_MODE_RAWEVENTS };
enum { VT_BLOCK, VT_NOBLOCK };

enum {
	IOCTL_VT_MODE, //VT_MODE_NORMAL/VT_MODE_EVENTS
	IOCTL_VT_BLOCKMODE, //VT_BLOCK/VT_NOBLOCK
};

/*struct lockkeystates_str{
	unsigned char scroll, caps, num;
};*/

struct vt_t {
	volatile int kb_buf[KB_BUFFER_SIZE];
	volatile int kb_buf_start, kb_buf_end, kb_buf_count;
	unsigned int realtime_kb_mods, kb_mods;
	//struct lockkeystates_str realtime_kb_locks, kb_locks;
	//struct spinlock kb_buf_lock;

	char * buffer;
	unsigned int bufw, bufh; //buffer w, h (actual width is w*2)
	int bufsize;
	int scroll; /* how many lines have we scrolled up */
	unsigned int cx, cy; /* cursor x, y */
	unsigned char color;
	struct spinlock printlock; /* we are going to get problems with these spinlocks, we should replace them with something better later */
	struct spinlock writelock;
	unsigned char in_kprintf;

	unsigned int num_open;

	unsigned char mode, block;
};

extern unsigned char vt_get_color(unsigned int vt_num);
extern void vt_set_color(unsigned int vt_num, unsigned char c);
extern void vt_getdisplaysize(unsigned int vt_num, unsigned int *w, unsigned int *h);
extern int vt_locate(unsigned int vt_num, unsigned int x, unsigned int y);
extern int vt_getpos(unsigned int vt_num, unsigned int *x, unsigned int *y);
extern void vt_cls(unsigned int vt_num);
//extern int vt_fastprint(unsigned int vt_num, const char *buf, unsigned int len);
extern int vt_print(unsigned int vt_num, const char *string);
extern int vt_putch(unsigned int vt_num, int c);
extern void vt_change(unsigned int vt_num);
extern void vt_scroll(int lines);
extern unsigned int vt_get_display_height(void);

extern unsigned int vt_out_get(void);
extern void vt_keyboard_event(int code, int up);
extern int vt_get_kbmods(unsigned int vt_num); //tämän antamat kbmodsit
                                //päivittyvät sitä mukaa kun vt:ltä luetaan
								//näppäimiä
extern int vt_get_next_key_event(unsigned int vt_num);
extern int vt_wait_and_get_next_key_event(unsigned int vt_num);
extern int vt_kb_get(unsigned int vt_num);
extern int vt_kb_peek(unsigned int vt_num);

extern void vt_kprintflock(unsigned int vt_num);
extern void vt_kprintfunlock(unsigned int vt_num);
extern void vt_unlockspinlocks(void);

extern void vt_init(void);
extern void vt_dev_init(void);
extern int vt_setdriver(char *filename);

//extern struct vt_t vt[VT_COUNT];
//extern unsigned int cur_vt;

#endif

