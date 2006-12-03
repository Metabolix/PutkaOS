#include <screen.h>
#include <stdarg.h>
#include <string.h>

#define BUF_KOKO 32
char sprintf_buf[BUF_KOKO+1]={0};

struct printf_format_tag {
	int left_align, always_sign, sharp;
	int fillchar, minwidth, minprec;
	int modifier;
} tag;

int abs_int(int x)
{
	if (x < 0) {
		return -x;
	}
	return x;
}

int sprintf_uint(unsigned int luku)
{
	int i = BUF_KOKO;
	if (!luku) {
		sprintf_buf[0] = '0';
		sprintf_buf[1] = 0;
		return 1;
	}
	while (luku) {
		sprintf_buf[--i] = (luku % 10) + '0';
		luku /= 10;
	}
	memmove(sprintf_buf, sprintf_buf + i, BUF_KOKO - i);
	sprintf_buf[BUF_KOKO - i] = 0;
	return BUF_KOKO - i;
}

int sprintf_uoct(unsigned int num)
{
	int i = BUF_KOKO;
	while (num) {
		sprintf_buf[--i] = (num & 0x07) + '0';
		num >>= 3;
	}


	if (tag.sharp) {
		memmove(sprintf_buf + 1, sprintf_buf + i, BUF_KOKO - i);
		sprintf_buf[0] = '0';
		sprintf_buf[BUF_KOKO + 1 - i] = 0;
		return BUF_KOKO + 1 - i;
	} else {
		memmove(sprintf_buf, sprintf_buf + i, BUF_KOKO - i);
		sprintf_buf[BUF_KOKO - i] = 0;
		return BUF_KOKO - i;
	}
}

int sprintf_hex(unsigned int num)
{
	const char convert[] = "0123456789abcdef";
	int i = BUF_KOKO;
	while (num) {
		sprintf_buf[--i] = convert[num & 0x0f];
		num >>= 4;
	}

	memmove(sprintf_buf, sprintf_buf + i, BUF_KOKO - i);
	sprintf_buf[BUF_KOKO - i] = 0;
	return BUF_KOKO - i;
}

int sprintf_heX(unsigned int num)
{
	const char convert[] = "0123456789ABCDEF";
	int i = BUF_KOKO;
	while (num) {
		sprintf_buf[--i] = convert[num & 0x0f];
		num >>= 4;
	}

	memmove(sprintf_buf, sprintf_buf + i, BUF_KOKO - i);
	sprintf_buf[BUF_KOKO - i] = 0;
	return BUF_KOKO - i;
}

int kprintf(const char *fmt, ...)
{
	int retval = 0, numbytes = 0, len, apu;
	union {
		char c;
		unsigned int ui;
		int i;
		int *ip;
		const char* s;
	} types;
	va_list args;
	va_start(args, fmt);
	while (*fmt) switch (*fmt) {
		case '%':
			memset(&tag, 0, sizeof(tag));
			tag.fillchar = ' ';
			flagisilmukka:
			++fmt;
			switch (*fmt) {
				case '-':
					tag.left_align = 1;
					goto flagisilmukka;
				case '+':
					tag.always_sign = '+';
					goto flagisilmukka;
				case ' ':
					tag.always_sign = ' ';
					goto flagisilmukka;
				case '#':
					tag.sharp = '#';
					goto flagisilmukka;
			}
			if (*fmt == '*') {
				tag.minwidth = va_arg(args, int);
				++fmt;
			} else {
				if (*fmt == '0') {
					tag.fillchar = '0';
					++fmt;
				}
				while (*fmt >= '0' && *fmt <= '9') {
					tag.minwidth = (10 * tag.minwidth) + (*fmt - '0');
					++fmt;
				}
			}
			if (*fmt == '.') {
				++fmt;
				while (*fmt >= '0' && *fmt <= '9') {
					tag.minprec = (10 * tag.minprec) + (*fmt - '0');
					++fmt;
				}
			}
			switch (*fmt) {
				case 'h':
				case 'l':
				case 'L':
					tag.modifier = *fmt;
					++fmt;
					break;
			}
			switch (*fmt) {
				case 'c':
					tag.fillchar = ' ';
					types.c = va_arg(args, char);
					len = 1;
					if (tag.left_align) {
						putch(types.c);
					}
					while (len < tag.minwidth) {
						putch(tag.fillchar);
						++len;
					}
					if (!tag.left_align) {
						putch(types.c);
					}
					numbytes += len;
					break;
				case 'u':
					tag.always_sign = 0;
				case 'd':
				case 'i':
					if (tag.modifier == 'h') {
						if (*fmt == 'u') {
							types.i = (unsigned short)va_arg(args, unsigned int);
						} else {
							types.i = (short)va_arg(args, int);
						}
					} else {
						types.i = va_arg(args, int);
					}
					if (*fmt == 'o') {
						len = sprintf_uoct(abs_int(types.i));
					} else if (*fmt == 'u') {
						len = sprintf_uint(types.ui);
					} else {
						len = sprintf_uint(abs_int(types.i));
					}
					if (types.i < 0) {
						tag.always_sign = '-';
					}
					if (tag.left_align) {
						tag.fillchar = ' ';
					}
					if (tag.fillchar == '0') {
						if (tag.always_sign) {
							putch(tag.always_sign);
							++len;
						}
						while (len < tag.minwidth || len < tag.minprec) {
							putch(tag.fillchar);
							++len;
						}
						print(sprintf_buf);
					} else {
						if (tag.left_align) {
							if (tag.always_sign) {
								putch(tag.always_sign);
								++len;
							}
							print(sprintf_buf);
							while (len < tag.minwidth || len < tag.minprec) {
								putch(tag.fillchar);
								++len;
							}
						} else {
							if (tag.always_sign) {
								++len;
							}
							while (len < tag.minwidth || len < tag.minprec) {
								putch(tag.fillchar);
								++len;
							}
							if (tag.always_sign) {
								putch(tag.always_sign);
							}
							print(sprintf_buf);
						}
					}
					numbytes += len;
					break;
				case 'x':
				case 'X':
					if (tag.modifier == 'h') {
						types.ui = (unsigned short)va_arg(args, unsigned int);
					} else {
						types.ui = va_arg(args, unsigned int);
					}
					if (*fmt == 'x') {
						len = sprintf_hex(types.ui);
					} else {
						len = sprintf_heX(types.ui);
					}
					if (tag.left_align) {
						tag.fillchar = ' ';
						if (tag.sharp) {
							putch('0');
							putch(*fmt);
							len += 2;
						}
						print(sprintf_buf);
						while (len < tag.minwidth || len < tag.minprec) {
							putch(tag.fillchar);
							++len;
						}
					} else {
						if (tag.fillchar == '0') {
							if (tag.sharp) {
								putch('0');
								putch(*fmt);
								len += 2;
							}
							while (len < tag.minwidth || len < tag.minprec) {
								putch(tag.fillchar);
								++len;
							}
							print(sprintf_buf);
						} else {
							if (tag.sharp) {
								len += 2;
							}
							while (len < tag.minwidth || len < tag.minprec) {
								putch(tag.fillchar);
								++len;
							}
							if (tag.sharp) {
								putch('0');
								putch(*fmt);
							}
							print(sprintf_buf);
						}
					}
					numbytes += len;
					break;
				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
					putch('[');
					putch(*fmt);
					print(" not implemented]");
					numbytes += 19;
					break;
				case 's':
					tag.fillchar = ' ';
					types.s = va_arg(args, const char*);
					len = strlen(types.s);
					if (tag.left_align) {
						print(types.s);
					}
					while (len < tag.minwidth) {
						putch(tag.fillchar);
						++len;
					}
					if (!tag.left_align) {
						print(types.s);
					}
					numbytes += len;
					break;
				case 'p':
					types.ui = va_arg(args, unsigned int);
					tag.fillchar = ' ';
					len = 9;
					if (!tag.left_align) while (len < tag.minwidth) {
						putch(tag.fillchar);
						++len;
					}
					apu = sprintf_heX(types.ui >> 16);
					while (apu < 4) {
						putch('0');
						++apu;
					}
					print(sprintf_buf);
					putch(':');
					apu = sprintf_heX(types.ui & 0xffff);
					while (apu < 4) {
						putch('0');
						++apu;
					}
					print(sprintf_buf);

					while (len < tag.minwidth) {
						putch(tag.fillchar);
						++len;
					}
					numbytes += len;
					break;
				case 'n':
					types.ip = va_arg(args, int*);
					*types.ip = numbytes;
					break;
				case '%':
					putch('%');
					++numbytes;
					break;
				default:
					print("[%");
					putch(*fmt);
					print(" O_o]");
					numbytes += 8;
			}
			++fmt;
			break;
		default:
			putch(*fmt);
			++fmt;
	}
	va_end(args);
	return retval;
}
