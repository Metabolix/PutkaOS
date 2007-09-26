#ifndef _SCREEN_H
#define _SCREEN_H

//Tämä ja screen.c ovat vain korvaamassa vanhoja vastaavia.
//Funktiot ainoastaan ohjailevat jutut oikeaan paikkaan

#include <malloc.h>

extern unsigned int vt_out_get(void);
extern void cls(void);
extern int print(const char * string);
extern int putch(char c);
extern int move(unsigned int y, unsigned int x);
extern unsigned char get_colour(void);
extern void set_colour(unsigned char c);

extern int kprintf(const char *fmt, ...);

#endif

