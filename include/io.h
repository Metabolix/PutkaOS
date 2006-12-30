#ifndef _IO_H
#define _IO_H
extern unsigned char inportb (unsigned short port);
extern unsigned short inportw (unsigned short port);
extern unsigned long inportdw (unsigned short port);

extern void outportb (unsigned short port, unsigned char data);
extern void outportw (unsigned short port, unsigned short data);
extern void outportdw (unsigned short port, unsigned long data);
#endif
