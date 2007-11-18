#ifndef _SCREEN_H
#define _SCREEN_H

//Tämä ja screen.c ovat vain korvaamassa vanhoja vastaavia.
//Funktiot ohjailevat jutut oikeaan paikkaan

//#include <memory/kmalloc.h> //useat screen.h:n includoivat eivät includoi tätä
#include <vt.h>

//#define cls() vt_cls((struct vt_file *) stderr)
#define cls() ioctl(stderr, IOCTL_VT_CLS, 0)
//#define syscall_print(string) vt_print((struct vt_file *) stderr, string)

#define putch(c) vt_putch((struct vt_file *) stderr, c)
//#define getdisplaysize(w, h) vt_getdisplaysize((struct vt_file *) stderr, w, h)
#define getdisplaysize(w, h) { unsigned int a[2]; \
                       ioctl(stderr, IOCTL_VT_GET_SIZE, (uintptr_t)&a);\
					   *w = a[0]; *h = a[1]; }
//#define locate(x, y) vt_locate((struct vt_file *) stderr, x, y)
#define locate(x, y) { unsigned int a[2]; a[0] = x; a[1] = y;\
                       ioctl(stderr, IOCTL_VT_SET_CURSOR_POS, (uintptr_t)&a); }
#define move(y, x) locate(x, y)
//#define getpos(x, y) vt_getpos((struct vt_file *) stderr, x, y)
#define getpos(x, y) { unsigned int a[2]; \
                       ioctl(stderr, IOCTL_VT_GET_CURSOR_POS, (uintptr_t)&a);\
					   *x = a[0]; *y = a[1]; }
//#define get_colour(c) vt_get_color((struct vt_file *) stderr)
#define get_colour(c) ioctl(stderr, IOCTL_VT_GET_COLOR, (uintptr_t)c)
//#define set_colour(c) vt_set_color((struct vt_file *) stderr, c)
#define set_colour(c) ioctl(stderr, IOCTL_VT_SET_COLOR, (uintptr_t)c)
#define get_next_key_event() vt_get_next_key_event((struct vt_file *) stdin)
#define wait_and_get_next_key_event() vt_wait_and_get_next_key_event((struct vt_file *) stdin)
#define kb_get() vt_kb_get((struct vt_file *) stdin)
#define kb_peek() vt_kb_peek((struct vt_file *) stdin)
#define print(x) fwrite(x, 1, strlen(x), stderr)

#include <kprintf.h>
//extern int kprintf(const char *fmt, ...);

#endif

