#ifndef _SCREEN_H
#define _SCREEN_H
void cls();
void print(const char * string);
void print_hex(unsigned int num);
void print_int(int luku);
void print_uint(unsigned int luku);
void move_cursor();
void putch(const char c);
int kprintf(const char *fmt, ...);
#endif

