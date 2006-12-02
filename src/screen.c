#include <screen.h>
#include <mem.h>
#include <io.h>
#include <stdarg.h>

unsigned ccol = 0, crow = 0; /* cursor row and col */

void cls()
{
	int a;
	for(a = 0; a < 25 * 80 * 2; a+=2) {
		*(char*)(0xB8000 + a) = ' ';
		*(char*)(0xB8000 + a + 1) = 0x7;
	}
	ccol = 0;
	crow = 0;
}
void print(const char * string)
{
	int a;
	for(a = 0; string[a] != 0; a++) {
		putch(string[a]);
	}
}

int kprintf(const char *fmt, ...)
{
	int retval = 0;
	union {
		char c;
		short hi;
		int i;
		const char* s;
	} types;
	va_list args;
	va_start(args, fmt);
	while (*fmt) switch (*fmt) {
		case '%':
			switch (fmt[1]) {
				case 'c':
					types.c = va_arg(args, char);
					putch(types.c);
					fmt += 2;
					break;
				case 'h':
					types.hi = va_arg(args, short);
					print_int(types.hi);
					fmt += 2;
					break;
				case 'u':
					types.i = va_arg(args, int);
					print_uint(types.i);
					fmt += 2;
					break;
				case 'i':
					types.i = va_arg(args, int);
					print_int(types.i);
					fmt += 2;
					break;
				case 'x':
					types.i = va_arg(args, int);
					print_hex(types.i);
					fmt += 2;
					break;
				case 's':
					types.s = va_arg(args, const char*);
					print(types.s);
					fmt += 2;
					break;
				case '%':
					putch('%');
					fmt += 2;
					break;
				default:
					putch(fmt[0]);
					putch(fmt[1]);
					fmt += 2;
			}
			break;
		default:
			putch(*fmt);
			++fmt;
	}
	va_end(args);
	return retval;
}

void putch(const char c) {
	if(c == '\b') { /* backspace */
		if(ccol > 0)
			ccol--;
	}
	else if(c == '\t') { /* tab */
		ccol = (ccol + 8) & ~7;
	}
	else if(c == '\r') { /* return */
		ccol = 0;
	}
	else if(c == '\n') { /* new line */
		ccol = 0;
		crow++;
	}
	else if(c >= ' ') { /* printable character */
		*(char*)(0xB8000 + crow * 160 + ccol * 2) = c;
		*(char*)(0xB8000 + crow * 160 + ccol * 2 + 1) = 0x07;
		ccol++;
	}

	if(ccol >= 80) {
		ccol = 0;
		crow++;
	}

	if(crow >= 25) { /* scroll screen */
		int amount = crow - 24;
		memcpy((void *)0xB8000, (void *)0xB8000 + amount * 160, (25 - amount) * 160);
		memset((void *)0xB8000 + (25 - amount) * 160, 0, 160);
		crow = 24;
	}
	move_cursor();
}

void move_cursor()
{
	unsigned int temp = crow * 80 + ccol;

	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

void print_uint(unsigned int luku)
{
	unsigned int a;
	if (!luku) {
		putch('0');
		return;
	}
	a = 1000000000UL;
	while (a > luku) {
		a /= 10;
	}
	while (a) {
		putch((luku / a) + '0');
		luku = luku % a;
		a /= 10;
	}
}

void print_int(int luku)
{
	if (luku < 0) {
		putch('-');
		print_uint(-luku);
	} else {
		print_uint(luku);
	}
}

void print_hex(unsigned int num)
{
	int length = 8;
	print("0x");
	const char convert[] = "0123456789abcdef";

	while(--length) { /* get the length */
		if(num >> (4 * length) & 0xF)
			break;
	}
	length++;

	while(length--) { /* output it! */
		int ch = (num >> (4 * length)) & 0xF;
		putch(convert[ch]);
	}
}

