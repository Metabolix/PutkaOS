#ifndef _SCREEN_H
#define _SCREEN_H

//Tämä ja screen.c ovat vain korvaamassa vanhoja vastaavia.
//Funktiot ohjailevat jutut oikeaan paikkaan

#include <memory/kmalloc.h> //useat screen.h:n includoivat eivät includoi tätä
#include <vt.h>

//extern void cls(void);
#define cls() vt_cls(vt_out_get())
extern int syscall_print(const char * string);
//#define syscall_print(string) vt_print(vt_out_get(), string)
//extern int putch(char c);
#define putch(c) vt_putch(vt_out_get(), c)
#define getdisplaysize(w, h) vt_getdisplaysize(vt_out_get(), w, h)
#define move(y, x) vt_locate(vt_out_get(), x, y)
#define locate(x, y) vt_locate(vt_out_get(), x, y)
#define getpos(x, y) vt_getpos(vt_out_get(), x, y)
//extern unsigned char get_colour(void);
#define get_colour(c) vt_get_color(vt_out_get())
//extern void set_colour(unsigned char c);
#define set_colour(c) vt_set_color(vt_out_get(), c)
#define get_next_key_event() vt_get_next_key_event(vt_out_get())
#define wait_and_get_next_key_event() vt_wait_and_get_next_key_event(vt_out_get())
#define get_kbmods() vt_get_kbmods(vt_out_get())
#define kb_get() vt_kb_get(vt_out_get())
#define kb_peek() vt_kb_peek(vt_out_get())

extern int kprintf(const char *fmt, ...);

#endif

