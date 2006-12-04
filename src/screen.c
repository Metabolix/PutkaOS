#include <screen.h>
#include <mem.h>
#include <io.h>

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
int print(const char * string)
{
	char *s = (char *)string;
	while (*s) {
		putch(*s);
		++s;
	}
	return s - string;
}

void putch(int c) {
	if(c == '\b') { /* backspace */
		if (ccol > 0) {
			ccol--;
		}
	}
	else if (c == '\t') { /* tab */
		ccol = (ccol + 8) & ~7;
	}
	else if (c == '\r') { /* return */
		ccol = 0;
	}
	else if (c == '\n') { /* new line */
		ccol = 0;
		crow++;
	}
	else if (c >= ' ') { /* printable character */
		*(char*)(0xB8000 + crow * 160 + ccol * 2) = c;
		*(char*)(0xB8000 + crow * 160 + ccol * 2 + 1) = 0x07;
		ccol++;
	}

	if (ccol >= 80) {
		ccol = 0;
		crow++;
	}

	if (crow >= 25) { /* scroll screen */
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

