#ifndef _SCREEN_H
#define _SCREEN_H

//Tämä ja screen.c ovat vain korvaamassa vanhoja vastaavia.
//Funktiot ohjailevat jutut oikeaan paikkaan

#include <memory/kmalloc.h> //useat screen.h:n includoivat eivät includoi tätä
#include <vt.h>

//extern void cls(void);
#define cls() vt_cls((struct vt_file *) stderr)
//#define syscall_print(string) vt_print((struct vt_file *) stderr, string)
//extern int putch(char c);

#define putch(c) vt_putch((struct vt_file *) stderr, c)
#define getdisplaysize(w, h) vt_getdisplaysize((struct vt_file *) stderr, w, h)
#define move(y, x) vt_locate((struct vt_file *) stderr, x, y)
#define locate(x, y) vt_locate((struct vt_file *) stderr, x, y)
#define getpos(x, y) vt_getpos((struct vt_file *) stderr, x, y)
//extern unsigned char get_colour(void);
#define get_colour(c) vt_get_color((struct vt_file *) stderr)
//extern void set_colour(unsigned char c);
#define set_colour(c) vt_set_color((struct vt_file *) stderr, c)
#define get_next_key_event() vt_get_next_key_event((struct vt_file *) stdin)
#define wait_and_get_next_key_event() vt_wait_and_get_next_key_event((struct vt_file *) stdin)
#define get_kbmods() vt_get_kbmods((struct vt_file *) stdin)
#define kb_get() vt_kb_get((struct vt_file *) stdin)
#define kb_peek() vt_kb_peek((struct vt_file *) stdin)
#define print(x) fwrite(x, 1, strlen(x), stderr)

#include <kprintf.h>
//extern int kprintf(const char *fmt, ...);

#endif

