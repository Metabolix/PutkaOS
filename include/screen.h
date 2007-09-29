#ifndef _SCREEN_H
#define _SCREEN_H

//Tämä ja screen.c ovat vain korvaamassa vanhoja vastaavia.
//Funktiot ainoastaan ohjailevat jutut oikeaan paikkaan

#include <malloc.h> //useat screen.h:n includoivat eivät includoi tätä
#include <vt.h>

extern unsigned int vt_out_get(void);
extern void cls(void);
extern int print(const char * string);
extern int putch(char c);
#define getdisplaysize(w, h) vt_getdisplaysize(vt_out_get(), w, h)
#define move(y, x) vt_locate(vt_out_get(), x, y)
#define locate(x, y) vt_locate(vt_out_get(), x, y)
#define getpos(x, y) vt_getpos(vt_out_get(), x, y)
extern unsigned char get_colour(void);
extern void set_colour(unsigned char c);
#define kb_get() vt_kb_get(vt_out_get())
#define kb_peek() vt_kb_peek(vt_out_get())

extern int kprintf(const char *fmt, ...);

#endif

