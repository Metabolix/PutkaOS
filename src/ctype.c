#include <ctype.h>

const int ctype_TRUE = (0 == 0);
const int ctype_FALSE = (0 != 0);

const unsigned char isblank_list[] = " \t";
const unsigned char isspace_list[] = " \n\r\t\f\v";
//const char isgraph_list[] = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

int ctype_check_list(const unsigned char *buf, int c)
{
	while (*buf) {
		if (c == *buf) {
			return ctype_TRUE;
		}
		++buf;
	}
	return ctype_FALSE;
}

int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

int isalpha(int c)
{
	return islower(c) || isupper(c);
}
#if 0
" !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"
0000:0000 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f ................
0000:0010 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f ................
0000:0020 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f  !"#$%&'()*+,-./
0000:0030 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f 0123456789:;<=>?
0000:0040 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f @ABCDEFGHIJKLMNO
0000:0050 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f PQRSTUVWXYZ[\]^_
0000:0060 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f `abcdefghijklmno
0000:0070 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f pqrstuvwxyz{|}~.
0000:0080 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f ................
0000:0090 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f ................
0000:00a0 a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af  ¡¢£¤¥¦§¨©ª«¬­®¯
0000:00b0 b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf °±²³´µ¶·¸¹º»¼½¾¿
0000:00c0 c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ
0000:00d0 d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß
0000:00e0 e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef àáâãäåæçèéêëìíîï
0000:00f0 f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff ðñòóôõö÷øùúûüýþÿ
0000:0100

#endif
int isdigit(int c)
{
	return ('0' <= c && c <= '9');
}

int isxdigit(int c)
{
	return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}

int islower(int c)
{
	return ('a' <= c && c <= 'z');
}

int isupper(int c)
{
	return ('A' <= c && c <= 'Z');
}

int isblank(int c)
{
	return ctype_check_list(isblank_list, c);
}

int iscntrl(int c)
{
	return !isgraph(c);
	// return ctype_check_list(iscntrl_list, c);
}

int isgraph(int c)
{
	return (0x20 <= c && c <= 0x7e);
	// return (c < 0x80) && (isalnum(c) || ctype_check_list(isgraph_list, c));
}

int isprint(int c)
{
	return isgraph(c) || (c == ' ');
}

int ispunct(int c)
{
	return !isalnum(c) && !isspace(c) && isprint(c);
}

int isspace(int c)
{
	return ctype_check_list(isspace_list, c);
}

int tolower(int c)
{
	if (!isupper(c)) return c;
	return c + ('a' - 'A');
}

int toupper(int c)
{
	if (!islower(c)) return c;
	return ((c - ('a' - 'A')) + 0x100) & 0xff;
}

