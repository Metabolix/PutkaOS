#ifndef _VT_H
#define _VT_H

#include <filesys/filesystem.h>
#include <spinlock.h>
#include <string.h>
#include <io.h>
#include <irq.h>
#include <multitasking/multitasking.h>
#include <memory/kmalloc.h>
#include <devices/devmanager.h>

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

enum { VT_MODE_NORMAL=0, VT_MODE_RAWEVENTS=1 };
/*
 * näppiseventit:
 * unsigned inttejä, 8. bitti (0x100) on ylhäällä jos nappi nousee ylös,
 * biteissä 0-7 (0xff) on keyboard.h:n KEYCODE-juttuja. Niitä voi muuttaa
 * asciiksi (tai joksikin sen tapaiseksi) keyboard.c/h:n key_to_asciilla.
 */

enum { VT_BLOCKMODE_NOBLOCK=0, VT_BLOCKMODE_BLOCK=1 };

enum {
	IOCTL_VT_MODE, //VT_MODE_NORMAL/VT_MODE_EVENTS
	IOCTL_VT_BLOCKMODE, //VT_NOBLOCK/VT_BLOCK
};

struct vt_t {
	DEVICE std;

	char name[5];
	unsigned int index;

	volatile int kb_buf[KB_BUFFER_SIZE];
	volatile int kb_buf_start, kb_buf_end, kb_buf_count;
	unsigned int realtime_kb_mods, kb_mods;
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

extern unsigned char vt_get_color(struct vt_t *vtptr);
extern void vt_set_color(struct vt_t *vtptr, unsigned char c);
extern void vt_getdisplaysize(struct vt_t *vtptr, unsigned int *w, unsigned int *h);
extern int vt_locate(struct vt_t *vtptr, unsigned int x, unsigned int y);
extern int vt_getpos(struct vt_t *vtptr, unsigned int *x, unsigned int *y);
extern void vt_cls(struct vt_t *vtptr);
//extern int vt_fastprint(struct vt_t *vtptr, const char *buf, unsigned int len);
extern int vt_print(struct vt_t *vtptr, const char *string);
extern int vt_putch(struct vt_t *vtptr, int c);
extern void vt_change(unsigned int vt_num);
extern void vt_scroll(int lines);
extern unsigned int vt_get_display_height(void);

extern struct vt_t * vt_out_get(void);
extern void vt_keyboard_event(int code, int up);
extern int vt_get_kbmods(struct vt_t *vtptr); //tämän antamat kbmodsit
                                //päivittyvät sitä mukaa kun vt:ltä luetaan
								//näppäimiä
extern int vt_get_next_key_event(struct vt_t *vtptr);
extern int vt_wait_and_get_next_key_event(struct vt_t *vtptr);
extern int vt_kb_get(struct vt_t *vtptr);
extern int vt_kb_peek(struct vt_t *vtptr);

/*extern void vt_kprintflock(struct vt_t *vtptr);
extern void vt_kprintfunlock(struct vt_t *vtptr);*/
extern void vt_unlockspinlocks(void);

extern void vt_init(void);
extern void vt_dev_init(void);
extern int vt_setdriver(char *filename);

#endif

