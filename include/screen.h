#ifndef _SCREEN_H
#define _SCREEN_H
extern void cls();
extern int print(const char * string);
extern void move_cursor();
extern void putch(int c);

extern int kprintf(const char *fmt, ...);

#endif

