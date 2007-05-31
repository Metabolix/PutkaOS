#include <filesys/file.h>
#include <stdarg.h>
#include <string.h>
#include <putkaos.h>
#include <stdint.h>

typedef uint32_t wint_t;

// TODO: jokainen fprintf_stub.

#define FPRINTF_GETARG_U(type) \
	((type)((sizeof(type) >= sizeof(unsigned int)) \
	? va_arg(args, type) \
	: va_arg(args, unsigned int)))

#define FPRINTF_GETARG_S(type) \
	((type)((sizeof(type) >= sizeof(int)) \
	? va_arg(args, type) \
	: va_arg(args, int)))

const char fprintf_hexmarks[] = "0123456789abcdef";
const char fprintf_heXmarks[] = "0123456789ABCDEF";
enum fmt_lenmod_t {
	LEN_def,
	LEN_hh,
	LEN_h,
	LEN_l,
	LEN_ll,
	LEN_imax,
	LEN_size,
	LEN_ptrdiff,
	LEN_longdouble
};
typedef enum fmt_lenmod_t fmt_lenmod_t;

struct printf_format_tag {
	int left_align, always_sign, sharp, fillchar;
	int width, precision;
	int has_width, has_precision;
	fmt_lenmod_t lenmod;
};

static int fprintf_stub(FILE *f, const char *what)
{
	const char *str1 = "[fprintf (";
	const char *str2 = "): STUB!]";
	return
		fwrite(str1, 1, strlen(str1), f) +
		fwrite(what, 1, strlen(what), f) +
		fwrite(str2, 1, strlen(str2), f);
}
/*
static int sprintf_uint(char *sprintf_buf, unsigned int num)
{
	int i = BUF_KOKO;
	if (!num) {
		sprintf_buf[0] = '0';
		sprintf_buf[1] = 0;
		return 1;
	}
	do {
		sprintf_buf[--i] = (num % 10) + '0';
		num /= 10;
	} while (num);
	memmove(sprintf_buf, sprintf_buf + i, BUF_KOKO - i);
	sprintf_buf[BUF_KOKO - i] = 0;
	return BUF_KOKO - i;
}

static int sprintf_uoct(char *sprintf_buf, unsigned int num)
{
	int i = BUF_KOKO;
	do {
		sprintf_buf[--i] = (num & 0x07) + '0';
		num >>= 3;
	} while (num);

	if (tag->sharp) {
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
*/

static char *fmt_uint_32(char * restrict bufend, uint32_t num)
{
	while (num) {
		*(--bufend) = '0' + (num % 10);
		num /= 10;
	}
	return bufend;
}
static char *fmt_uint_64(char * restrict bufend, uint64_t num)
{
	char * newend = bufend;
	while (num) {
		uint64_t rem;
		uint32_t num32;
		while (newend > bufend) {
			*(--newend) = '0';
		}
		if ((num32 = num) == num) {
			num = 0;
		} else {
			num = uint64_div_rem(num, 1000000000, &rem);
			num32 = rem;
		}
		newend = fmt_uint_32(bufend, num32);
		bufend -= 9;
	}
	return newend;
}

static char *fmt_int_64(char * restrict bufend, int64_t num)
{
	if (num < 0) {
		bufend = fmt_uint_64(bufend, -num);
		*(--bufend) = '-';
	} else {
		bufend = fmt_uint_64(bufend, num);
	}
	return bufend;
}
static char *fmt_int_32(char * restrict bufend, int32_t num)
{
	if (num < 0) {
		bufend = fmt_uint_32(bufend, -num);
		*(--bufend) = '-';
	} else {
		bufend = fmt_uint_32(bufend, num);
	}
	return bufend;
}

static char *fmt_hex_64(char * restrict bufend, uint64_t num, int bigtxt)
{
	uint32_t shl;
	const char *merkit = ((bigtxt && bigtxt != 'x') ? fprintf_heXmarks : fprintf_hexmarks);
	bufend[-1] = merkit[0];
	for (shl = 0; num >> shl; shl += 4) {
		*(--bufend) = merkit[(uint32_t)(num >> shl) & 0x0f];
	}
	return bufend;
}
static char *fmt_hex_32(char * restrict bufend, uint32_t num, int bigtxt)
{
	uint32_t shl;
	const char *merkit = ((bigtxt && bigtxt != 'x') ? fprintf_heXmarks : fprintf_hexmarks);
	bufend[-1] = merkit[0];
	for (shl = 0; num >> shl; shl += 4) {
		*(--bufend) = merkit[(uint32_t)(num >> shl) & 0x0f];
	}
	return bufend;
}

static char *fmt_oct_64(char * restrict bufend, uint64_t num)
{
	uint32_t shl;
	bufend[-1] = '0';
	for (shl = 0; num >> shl; shl += 3) {
		*(--bufend) = '0' + ((uint32_t)(num >> shl) & 0x07);
	}
	return bufend;
}
static char *fmt_oct_32(char * restrict bufend, uint32_t num)
{
	uint32_t shl;
	bufend[-1] = '0';
	for (shl = 0; num >> shl; shl += 3) {
		*(--bufend) = '0' + ((uint32_t)(num >> shl) & 0x07);
	}
	return bufend;
}

static int fprintf_heX(FILE *f, struct printf_format_tag *tag, uintmax_t num)
{
	char buf[32], *ptr;
	buf[31] = 0;
	ptr = fmt_hex_64(buf + 31, num, 'X');
	*(--ptr) = ' ';
	*(--ptr) = 'X';
	*(--ptr) = 'e';
	*(--ptr) = 'h';
	return fprintf_stub(f, ptr);
}

static int fprintf_hex(FILE *f, struct printf_format_tag *tag, uintmax_t num)
{
	char buf[32], *ptr;
	buf[31] = 0;
	ptr = fmt_hex_64(buf + 31, num, 'x');
	*(--ptr) = ' ';
	*(--ptr) = 'x';
	*(--ptr) = 'e';
	*(--ptr) = 'h';
	return fprintf_stub(f, ptr);
}

static int fprintf_oct(FILE *f, struct printf_format_tag *tag, uintmax_t num)
{
	char buf[32], *ptr;
	buf[31] = 0;
	ptr = fmt_oct_64(buf + 31, num);
	*(--ptr) = ' ';
	*(--ptr) = 't';
	*(--ptr) = 'c';
	*(--ptr) = 'o';
	return fprintf_stub(f, ptr);
}

static int fprintf_uint(FILE *f, struct printf_format_tag *tag, uintmax_t num)
{
	char buf[32], *ptr;
	buf[31] = 0;
	ptr = fmt_uint_64(buf + 31, num);
	*(--ptr) = ' ';
	*(--ptr) = 't';
	*(--ptr) = 'n';
	*(--ptr) = 'i';
	*(--ptr) = 'u';
	return fprintf_stub(f, ptr);
}

static int fprintf_int(FILE *f, struct printf_format_tag *tag, intmax_t num)
{
	char buf[32], *ptr;
	buf[31] = 0;
	ptr = fmt_int_64(buf + 31, num);
	*(--ptr) = ' ';
	*(--ptr) = 't';
	*(--ptr) = 'n';
	*(--ptr) = 'i';
	return fprintf_stub(f, ptr);
}

static int fprintf_fnan(FILE *f, struct printf_format_tag *tag)
{
	return fprintf_stub(f, "fnan");
}
static int fprintf_finf(FILE *f, struct printf_format_tag *tag, int sign)
{
	return fprintf_stub(f, "finf");
}
static int fprintf_fzero(FILE *f, struct printf_format_tag *tag, int sign)
{
	return fprintf_stub(f, "fzero");
}

static long int fprintf_longdouble(FILE *f, struct printf_format_tag *tag, long double val)
{
	const long double inf = 1.0 / 0.0;
	const long double ninf = 1.0 / 0.0;
	const long double zero = 0.0;
	const long double nzero = -0.0;
#if 0
a \in [1, 2[
c \in [1, 10[

f = c * 10^d

log10(f) = d + log10(c)
d = floor(log10(f))
c = f / 10^d
#endif

	if (val != val) { // NAN
		return fprintf_fnan(f, tag);
	}
	if (val == inf) {
		return fprintf_finf(f, tag, 1);
	}
	if (val == ninf) {
		return fprintf_finf(f, tag, -1);
	}
	if (memcmp(&val, &zero, sizeof(val)) == 0) {
		return fprintf_fzero(f, tag, 1);
	}
	if (memcmp(&val, &nzero, sizeof(val)) == 0) {
		return fprintf_fzero(f, tag, -1);
	}

	// TODO: jotain kunnollista. ;)
	const uint32_t max_len = 128;
	char buf[max_len], str[max_len];
	char *ptr;

	str[0] = 0;
	if (val < 0) {
		val = -val;
		strcat(str, "-");
	}

	int ep = 0;
	while (val >= 10.0) {
		++ep;
		val /= 10.0;
	}
	while (val < 1.0) {
		--ep;
		val *= 10.0;
	}

	ptr = fmt_uint_32(buf + max_len, (10000 * val) + 0.5);
	--ptr;
	ptr[0] = ptr[1];
	ptr[1] = '.';
	strcpy(str, ptr);

	if (ep) {
		ptr = fmt_int_32(buf + max_len, ep);
		--ptr;
		ptr[0] = 'e';
		strcat(str, ptr);
	}

	return fwrite(str, 1, strlen(str), f);
}
static int fprintf_double(FILE *f, struct printf_format_tag *tag, double val)
{
	// TODO
	return fprintf_longdouble(f, tag, val);
}

static int fprintf_double_short(FILE *f, struct printf_format_tag *tag, double d, char bigchar)
{
	// TODO
	return fprintf_double(f, tag, d);
}
static int fprintf_longdouble_short(FILE *f, struct printf_format_tag *tag, long double d, char bigchar)
{
	// TODO
	return fprintf_longdouble(f, tag, d);
}

static int fprintf_double_exp(FILE *f, struct printf_format_tag *tag, double d, char bigchar)
{
	// TODO
	return fprintf_double(f, tag, d);
}
static int fprintf_longdouble_exp(FILE *f, struct printf_format_tag *tag, long double d, char bigchar)
{
	// TODO
	return fprintf_longdouble(f, tag, d);
}

static int fprintf_ptr(FILE *f, struct printf_format_tag *tag, uintptr_t addr)
{
	tag->fillchar = ' ';
	int ret = 0, len = 12; // [12345678xP]
	char buf[13] = "[00000000xP]";

	if (!tag->left_align) while (len < tag->width) {
		if (!fwrite(&tag->fillchar, 1, 1, f)) {
			break;
		}
		++ret; ++len;
	}
	fmt_hex_32(buf + 9, addr, 0);
	ret += fwrite(buf, 1, 12, f);

	if (tag->left_align) while (len < tag->width) {
		if (!fwrite(&tag->fillchar, 1, 1, f)) {
			break;
		}
		++ret; ++len;
	}
	return ret;
}

static int fprintf_char(FILE *f, struct printf_format_tag *tag, unsigned char c)
{
	int len = 1;
	if (tag->left_align) {
		fwrite(&c, 1, 1, f);
	}
	while (len < tag->width) {
		fwrite(&tag->fillchar, 1, 1, f);
		++len;
	}
	if (!tag->left_align) {
		fwrite(&c, 1, 1, f);
	}
	return len;
}
static int fprintf_str(FILE *f, struct printf_format_tag *tag, const char *str)
{
	if (!str) {
		return 6;
	}
	int len, slen, i;
	len = slen = strlen(str);
	if (tag->left_align) {
		if ((i = fwrite(str, slen, 1, f)) != slen) {
			return i;
		}
	}
	while (len < tag->width) {
		if (!fwrite(&tag->fillchar, 1, 1, f)) {
			break;
		}
		++len;
	}
	if (!tag->left_align) {
		if ((i = fwrite(str, slen, 1, f)) != slen) {
			return i;
		}
	}
	return len;
}

static int fprintf_wchar(FILE *f, struct printf_format_tag *tag, wchar_t c)
{
	// TODO!
	return fprintf_stub(f, "wchar");
}

static int fprintf_wstr(FILE *f, struct printf_format_tag *tag, const wchar_t *str)
{
	// TODO!
	return fprintf_stub(f, "wstr");
}

static void fprintf_numbytes(void *ptr, struct printf_format_tag *tag, int numbytes)
{
	switch (tag->lenmod) {
		case LEN_hh:
			*(signed char*)ptr = numbytes;
			break;
		case LEN_h:
			*(signed short*)ptr = numbytes;
			break;
		case LEN_l:
			*(signed long*)ptr = numbytes;
			break;
		case LEN_ll:
			*(signed long long*)ptr = numbytes;
			break;
		case LEN_imax:
			*(intmax_t*)ptr = numbytes;
			break;
		case LEN_size:
			*(size_t*)ptr = numbytes;
			break;
		case LEN_ptrdiff:
			*(ptrdiff_t*)ptr = numbytes;
			break;
		default:
			*(int*)ptr = numbytes;
			break;
	}
}

static const char *fprintf_lenmod(const char * const fmt, fmt_lenmod_t *lenmod)
{
	switch (*fmt) {
	case 'h':
		if (fmt[1] == 'h') {
			*lenmod = LEN_hh;
			return fmt + 2;
		}
		*lenmod = LEN_h;
		return fmt + 1;

	case 'l':
		if (fmt[1] == 'l') {
			*lenmod = LEN_ll;
			return fmt + 2;
		}
		*lenmod = LEN_l;
		return fmt + 1;

	case 'j':
		*lenmod = LEN_imax;
		return fmt + 1;
	case 'z':
		*lenmod = LEN_size;
		return fmt + 1;
	case 't':
		*lenmod = LEN_ptrdiff;
		return fmt + 1;
	case 'L':
		*lenmod = LEN_longdouble;
		return fmt + 1;
	}
	return fmt;
}

static const char *fprintf_flags(const char *fmt, struct printf_format_tag *tag)
{
	do { switch (*fmt) {
		case '-':
			tag->left_align = 1;
			break;
		case '+':
			tag->always_sign = '+';
			break;
		case ' ':
			if (!tag->always_sign) {
				tag->always_sign = ' ';
			}
			break;
		case '#':
			tag->sharp = '#';
			break;
		case '0':
			// diouxXaAeEfFgG
			tag->fillchar = '0';
			break;
		default:
			if (tag->left_align) {
				tag->fillchar = ' ';
			}
			return fmt;
	} } while (++fmt);
	return fmt;
}

#include <screen.h>
int fprintf(FILE * restrict f, const char * restrict fmt, ...)
{
	int retval = 0, numbytes = 0;
	const char *strptr;
	//kprintf("fprintf: %p %p %s\n", f, fmt, fmt);

	union types_t {
	/*
		unsigned char uc;
		char c;
		unsigned long long ulli;
		long long lli;
		unsigned long uli;
		long li;
		unsigned int ui;
		int i;
		unsigned short usi;
		short si;
		int *ip;
		const char* s;
		float flo;
		double dbl;
		*/
		intmax_t smax;
		uintmax_t umax;
	};
	union types_t types;
	struct printf_format_tag rtag, * const tag = &rtag;

	va_list args;
	va_start(args, fmt);
	while (*fmt) {
		for (strptr = fmt; *strptr && *strptr != '%'; ++strptr);
		if (!strptr[0]) {
			numbytes += fwrite(fmt, 1, strptr - fmt, f);
			break;
		}

		// "abc%%..." => print("abc%"), fmt = "..."
		if (strptr[1] == '%') {
			numbytes += fwrite(fmt, 1, strptr - fmt + 1, f);
			fmt = strptr + 2;
			continue;
		}
		numbytes += fwrite(fmt, 1, strptr - fmt, f);

		// fmt = "%...?"
		fmt = strptr;
		++strptr;

		memset(tag, 0, sizeof(tag));
		tag->fillchar = ' ';
		tag->lenmod = LEN_def;

		// Flagit: tasaus, etumerkki (+ ), pad-0, 0x
		strptr = fprintf_flags(strptr, tag);

		// Minimileveys, parametrina tai tästä numeroin
		if (*strptr == '*') {
			tag->width = va_arg(args, int);
			++strptr;
		} else {
			while (*strptr >= '0' && *strptr <= '9') {
				tag->width = (10 * tag->width) + (*strptr - '0');
				tag->has_width = 1;
				++strptr;
			}
		}
		// Tarkkuus liukuluvulla, muilla vaihtoehtoinen width
		if (*strptr == '.') {
			++strptr;
			if (*strptr == '*') {
				tag->precision = va_arg(args, int);
				++strptr;
			} else {
				while (*strptr >= '0' && *strptr <= '9') {
					tag->precision = (10 * tag->precision) + (*strptr - '0');
					tag->has_precision = 1;
					++strptr;
				}
			}
		}
		strptr = fprintf_lenmod(strptr, &tag->lenmod);

		switch (*strptr) {
			case 'c':
				if (tag->lenmod == LEN_l) {
					numbytes += fprintf_wchar(f, tag, va_arg(args, wint_t));
				} else {
					numbytes += fprintf_char(f, tag, va_arg(args, int));
				}
				break;
			case 'e':
			case 'E':
				if (tag->lenmod == LEN_longdouble) {
					numbytes += fprintf_longdouble_exp(f, tag, va_arg(args, long double), *strptr);
				} else {
					numbytes += fprintf_double_exp(f, tag, va_arg(args, double), *strptr);
				}
				break;
			case 'f':
				if (tag->lenmod == LEN_longdouble) {
					numbytes += fprintf_longdouble(f, tag, va_arg(args, long double));
				} else {
					numbytes += fprintf_double(f, tag, va_arg(args, double));
				}
				break;
			case 'g':
			case 'G':
				if (tag->lenmod == LEN_longdouble) {
					numbytes += fprintf_longdouble_short(f, tag, va_arg(args, long double), *strptr);
				} else {
					numbytes += fprintf_double_short(f, tag, va_arg(args, double), *strptr);
				}
				break;
			case 's':
				tag->fillchar = ' ';
				if (tag->lenmod == LEN_l) {
					numbytes += fprintf_wstr(f, tag, va_arg(args, const wchar_t*));
				} else {
					numbytes += fprintf_str(f, tag, va_arg(args, const char*));
				}
				break;
			case 'p':
				numbytes += fprintf_ptr(f, tag, va_arg(args, uintptr_t));
				break;
			case 'n':
				fprintf_numbytes(va_arg(args, void*), tag, numbytes);
				break;
			case 'd':
			case 'i':
				switch (tag->lenmod) {
					case LEN_hh:
						types.smax = FPRINTF_GETARG_S(signed char);
						break;
					case LEN_h:
						types.smax = FPRINTF_GETARG_S(signed short);
						break;
					case LEN_l:
						types.smax = FPRINTF_GETARG_S(signed long int);
						break;
					case LEN_size:
						types.smax = (signed) FPRINTF_GETARG_S(size_t);
						break;
					case LEN_ptrdiff:
						types.smax = (signed) FPRINTF_GETARG_S(ptrdiff_t);
						break;
					case LEN_ll:
						types.smax = FPRINTF_GETARG_S(signed long long int);
						break;
					case LEN_imax:
						types.smax = FPRINTF_GETARG_S(intmax_t);
						break;
					default:
						types.smax = FPRINTF_GETARG_S(signed int);
						break;
				}
				numbytes += fprintf_int(f, tag, types.smax);
				break;

			case 'u':
			case 'o':
			case 'x':
			case 'X':
				switch (tag->lenmod) {
					case LEN_hh:
						types.umax = FPRINTF_GETARG_U(unsigned char);
						break;
					case LEN_h:
						types.umax = FPRINTF_GETARG_U(unsigned short);
						break;
					case LEN_l:
						types.umax = FPRINTF_GETARG_U(unsigned long int);
						break;
					case LEN_size:
						types.umax = (unsigned) FPRINTF_GETARG_U(size_t);
						break;
					case LEN_ptrdiff:
						types.umax = (unsigned) FPRINTF_GETARG_U(ptrdiff_t);
						break;
					case LEN_ll:
						types.umax = FPRINTF_GETARG_U(unsigned long long int);
						break;
					case LEN_imax:
						types.umax = FPRINTF_GETARG_U(uintmax_t);
						break;
					default:
						types.umax = FPRINTF_GETARG_U(unsigned int);
						break;
				}
				switch (*strptr) {
					case 'u':
						numbytes += fprintf_uint(f, tag, types.umax);
						break;
					case 'o':
						numbytes += fprintf_oct(f, tag, types.umax);
						break;
					case 'x':
						numbytes += fprintf_hex(f, tag, types.umax);
						break;
					case 'X':
						numbytes += fprintf_heX(f, tag, types.umax);
						break;
				}
				break;
#if 0
			case 'u': // unsigned -> ei kuulu pakostakaan etumerkkiä
			case 'o': // samoin oktaali TODO!
				tag->always_sign = 0;
			case 'd':
			case 'i':
				if (tag->modifier == 'h') {
					if (*strptr == 'u' || *strptr == 'o') {
						types.i = (unsigned short)va_arg(args, unsigned int);
					} else {
						types.i = (short)va_arg(args, int);
					}
				} else {
					types.i = va_arg(args, int);
				}
				// Etumerkkikö?
				if (*strptr != 'u' && *strptr != 'o' && types.i < 0) {
					tag->always_sign = '-';
					types.i = -types.i;
				}
				// Printtaus sopivaksi
				if (*strptr == 'o') {
					len = sprintf_uoct(types.i);
				} else {
					len = sprintf_uint(types.i);
				}

				if (tag->left_align) {
					// Left-alignattua ei voi paddata nollilla
					tag->fillchar = ' ';
					// Merkki
					if (tag->always_sign) {
						putch(tag->always_sign);
						++len;
					}
					// Luku
					print(sprintf_buf);
					// Paddaus
					while (len < tag->width || len < tag->minprec) {
						putch(tag->fillchar);
						++len;
					}
				} else {
					if (tag->fillchar == '0') {
						// Ensin merkki
						if (tag->always_sign) {
							putch(tag->always_sign);
							++len;
						}
						// Paddaus
						while (len < tag->width || len < tag->minprec) {
							putch(tag->fillchar);
							++len;
						}
						// Luku
						print(sprintf_buf);
					} else {
						// Merkki mukaan pituuteen
						if (tag->always_sign) {
							++len;
						}
						// Paddaus
						while (len < tag->width || len < tag->minprec) {
							putch(tag->fillchar);
							++len;
						}
						// Merkki
						if (tag->always_sign) {
							putch(tag->always_sign);
						}
						// Luku
						print(sprintf_buf);
					}
				}
				numbytes += len;
				break;
			case 'x':
			case 'X':
				if (tag->modifier == 'h') {
					types.ui = (unsigned short)va_arg(args, unsigned int);
				} else {
					types.ui = va_arg(args, unsigned int);
				}
				if (*strptr == 'x') {
					len = sprintf_hex(types.ui);
				} else {
					len = sprintf_heX(types.ui);
				}
				if (tag->left_align) {
					tag->fillchar = ' ';
					if (tag->sharp) {
						putch('0');
						putch(*strptr);
						len += 2;
					}
					print(sprintf_buf);
					while (len < tag->width || len < tag->minprec) {
						putch(tag->fillchar);
						++len;
					}
				} else {
					if (tag->fillchar == '0') {
						if (tag->sharp) {
							putch('0');
							putch(*strptr);
							len += 2;
						}
						while (len < tag->width || len < tag->minprec) {
							putch(tag->fillchar);
							++len;
						}
						print(sprintf_buf);
					} else {
						if (tag->sharp) {
							len += 2;
						}
						while (len < tag->width || len < tag->minprec) {
							putch(tag->fillchar);
							++len;
						}
						if (tag->sharp) {
							putch('0');
							putch(*strptr);
						}
						print(sprintf_buf);
					}
				}
				numbytes += len;
				break;
#endif
			default:
				--retval;
				numbytes += fwrite("[", 1, 1, f);
				numbytes += fwrite(fmt, 1, strptr - fmt + 1, f);
				numbytes += fwrite(" O_o]", 1, 5, f);
		}
		++retval;
		++strptr;
		fmt = strptr;
	}
	va_end(args);

	return retval;
}
