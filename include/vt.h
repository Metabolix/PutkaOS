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
#define VT_KERN_LOG 0 //kernelin jutut menee tänne (tai ainakin pitäisi)

#define VT_KB_BUFFER_SIZE 128
#define VT_KB_QUEUE_SIZE 11
#define VT_ANSIBUF_SIZE 10+1

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

enum { VT_MODE_OLD=0, VT_MODE_NORMAL, VT_MODE_RAWEVENTS };
/*
 * näppiseventit:
 * unsigned inttejä, 8. bitti (0x100) on ylhäällä jos nappi nousee ylös,
 * biteissä 0-7 (0xff) on keyboard.h:n KEYCODE-juttuja. Niitä voi muuttaa
 * asciiksi (tai joksikin sen tapaiseksi) keyboard.c/h:n key_to_asciilla.
 */

enum { VT_BLOCKMODE_NOBLOCK=0, VT_BLOCKMODE_BLOCK };

enum {
	IOCTL_VT_READMODE, //VT_MODE_NORMAL/VT_MODE_EVENTS
	IOCTL_VT_BLOCKMODE, //VT_NOBLOCK/VT_BLOCK
	IOCTL_VT_SET_COLOR,
	IOCTL_VT_GET_COLOR,
	IOCTL_VT_ANSICODES_ENABLE, //param 0 = disable, 1 = enable (tulostuksessa)
	IOCTL_VT_GET_SIZE, //param. unsigned int [2] -taulukko. [0] = w, [1] = h
	IOCTL_VT_SET_CURSOR_POS, //parametrina unsigned int [2] -taulukko (x,y)
	IOCTL_VT_GET_CURSOR_POS, //parametrina unsigned int [2] -taulukko
	IOCTL_VT_CLS,
	IOCTL_VT_GET_KBMODS, //ks. keyboard.c/h
};

struct vt {
	DEVICE std;

	char name[5];
	unsigned int index;

	volatile int kb_buf[VT_KB_BUFFER_SIZE];
	struct spinlock kb_buf_lock;
	volatile int kb_buf_start, kb_buf_end, kb_buf_count;
	unsigned int realtime_kb_mods, kb_mods;
	char kb_queue[VT_KB_QUEUE_SIZE];
	struct spinlock queuelock;
	unsigned int kb_queue_start, kb_queue_count, kb_queue_end;

	char * buffer;
	unsigned int bufw, bufh; //buffer w, h (actual width is w*2)
	int bufsize;
	int scroll; /* how many lines have we scrolled up */
	unsigned int cx, cy; /* cursor x, y */
	struct spinlock printlock; /* we are going to get problems with these spinlocks, we should replace them with something better later */
	struct spinlock writelock;

	unsigned int num_open;

	unsigned char mode, block, ansicodes_enable;
};

struct vt_file {
	FILE std;
	struct vt *vtptr;
	unsigned char color;
	char ansibuf[VT_ANSIBUF_SIZE];
	unsigned int ansibuf_count;
	unsigned int ansi_coming, ansi_param_index, ansi_params_sgr[256];
	int ansi_params[2];
};

//extern unsigned char vt_get_color(struct vt_file *f);
//extern void vt_set_color(struct vt_file *f, unsigned char c);
//extern void vt_getdisplaysize(struct vt_file *f, unsigned int *w, unsigned int *h);
//extern int vt_locate(struct vt_file *f, unsigned int x, unsigned int y);
//extern int vt_getpos(struct vt_file *f, unsigned int *x, unsigned int *y);
//extern void vt_cls(struct vt_file *f);
//extern size_t vt_print(struct vt_file *f, const char *string);
extern int vt_putch(struct vt_file *f, int c);

extern void vt_change(unsigned int vt_num);
extern void vt_scroll(int lines);
extern unsigned int vt_get_display_height(void);

extern void vt_keyboard_event(int code, int up);

/**
* Tämän antamat kbmodsit päivittyvät sitä mukaa, kun vt:ltä luetaan näppäimiä
**/
//extern int vt_get_kbmods(struct vt_file *f);
//extern int vt_get_next_key_event(struct vt_file *f);
//extern int vt_wait_and_get_next_key_event(struct vt_file *f);
//extern int vt_kb_get(struct vt_file *f);
//extern int vt_kb_peek(struct vt_file *f);

extern void vt_unlockspinlocks(void);

extern void vt_init(void);
extern void vt_dev_init(void);
extern int vt_setdriver(char *filename);

#endif

