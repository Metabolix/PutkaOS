#include <inttypes.h>
#include <stdint.h>
#include <ctype.h>

intmax_t imaxabs(intmax_t j)
{
	return (j < 0) ? -j : j;
}

imaxdiv_t imaxdiv(intmax_t yla, intmax_t ala) // TODO
{
	int i = 0, j = 0;
	i /= j;
}

uintmax_t _strtoumax_sgn(const char * restrict nptr, char ** restrict end, int base, int * restrict negative)
{
	const char * restrict str, * restrict p;
	str = nptr;
	int minus = 0;
	uintmax_t val = 0;
	int i;
	if (!str || base < 0 || base == 1 || base > 36) {
		goto error_invalid;
	}
	while (isspace(*str)) {
		++str;
	}
	if (str[0] == '-') {
		minus = -1;
		++str;
	} else {
		minus = 0;
		if (str[0] == '+') {
			++str;
		}
	}
	if (!base) {
		if (str[0] == '0') {
			if (str[1] == 'x' || str[1] == 'X') {
				str += 16;
				base = 16;
			} else {
				++str;
				base = 8;
			}
		} else {
			base = 10;
		}
	} else if (base == 16 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		str += 2;
	}
	while (*str == '0') {
		++str;
	}
	p = str;
	switch (base) {
		case 2:
			while (*p == '0' || *p == '1') {
				++p;
			}
			if (p - str > 8 * sizeof(uintmax_t)) {
				str = p;
				goto return_overflow;
			}
			--p;
			while (p > str) {
				val |= (uintmax_t)(*str - '0') << (p - str);
				++str;
			}
			goto return_etc;
		case 4:
			while ('0' <= *p && *p <= '3') {
				++p;
			}
			if (p - str > 4 * sizeof(uintmax_t)) {
				str = p;
				goto return_overflow;
			}
			--p;
			while (p > str) {
				val |= (uintmax_t)(*str - '0') << (2 * (p - str));
				++str;
			}
			goto return_etc;
		case 8:
			while ('0' <= *p && *p <= '7') {
				++p;
			}
			if (p - str > (8 * sizeof(uintmax_t) + 2) / 3) {
				str = p;
				goto return_overflow;
			}
			if (p - str > (8 * sizeof(uintmax_t)) / 3) {
				i = (p[-1] - '0');
				if (i > (UINTMAX_MAX >> (3 * (p - 1 - str)))) {
					str = p;
					goto return_overflow;
				}
			}
			--p;
			while (p > str) {
				val |= (uintmax_t)(*str - '0') << (3 * (p - str));
				++str;
			}
			goto return_etc;
		case 16:
			while (('0' <= *p && *p <= '9') || ('a' <= *p && *p <= 'f') || ('A' <= *p && *p <= 'F')) {
				++p;
			}
			if (p - str > 2 * sizeof(uintmax_t)) {
				str = p;
				goto return_overflow;
			}
			--p;
			while (p >= str) {
				if ('0' <= *str && *str <= '9') {
					val |= (uintmax_t)(*str - '0') << (4 * (p - str));
				} else if ('a' <= *str && *str <= 'f') {
					val |= (uintmax_t)(10 + *str - 'a') << (4 * (p - str));
				} else /* A-Z */{
					val |= (uintmax_t)(10 + *str - 'A') << (4 * (p - str));
				}
				++str;
			}
			goto return_etc;
		case 32:
			while (('0' <= *p && *p <= '9') || ('a' <= *p && *p <= 'v') || ('A' <= *p && *p <= 'V')) {
				++p;
			}
			if (p - str > (8 * sizeof(uintmax_t) + 4) / 5) {
				str = p;
				goto return_overflow;
			}
			if (p - str > 8 * sizeof(uintmax_t) / 5) {
				if ('0' <= p[-1] && p[-1] <= '9') {
					i = (p[-1] - '0');
				} else if ('a' <= p[-1] && p[-1] <= 'v') {
					i = (10 + p[-1] - 'a');
				} else /* A-Z */{
					i = (10 + p[-1] - 'A');
				}
				if (i > (UINTMAX_MAX >> (5 * (p - 1 - str)))) {
					str = p;
					goto return_overflow;
				}
			}
			--p;
			while (p > str) {
				if ('0' <= *str && *str <= '9') {
					val |= (uintmax_t)(*str - '0') << (5 * (p - str));
				} else if ('a' <= *str && *str <= 'v') {
					val |= (uintmax_t)(10 + *str - 'a') << (5 * (p - str));
				} else /* A-Z */{
					val |= (uintmax_t)(10 + *str - 'A') << (5 * (p - str));
				}
				++str;
			}
			goto return_etc;
		default:
			if (base <= 10) {
				goto le10;
			}
			goto gt10;
	}
le10:
	while ('0' <= *p && *p < ('0' + base)) {
		val = (base * val) + (*p - '0');
		++p;
	}
	// TODO: overflow
	str = p;
	goto return_etc;
gt10:
	while (1) {
		if ('0' <= *p && *p <= '9') {
			val = (base * val) + (*p - '0');
		} else if (*p <= 'a' && (*p - 'a') < base - 10) {
			val = (base * val) + (*p + 10 - 'a');
		} else if (*p <= 'A' && (*p - 'A') < base - 10) {
			val = (base * val) + (*p + 10 - 'A');
		} else {
			break;
		}
		++p;
	}
	// TODO: overflow
	str = p;
	goto return_etc;

return_overflow:
	val = UINTMAX_MAX;
return_etc:
	if (negative) {
		*negative = minus;
	}
	if (end) {
		*end = (char *) str;
	}
	return val;
error_invalid:
	if (end) {
		*end = (char *) nptr;
	}
	return 0;
}

uintmax_t strtoumax(const char * restrict nptr, char ** restrict end, int base)
{
	int sgn;
	uintmax_t val;
	val = _strtoumax_sgn(nptr, end, base, &sgn);
	if (sgn) {
		return UINTMAX_MAX;
	}
	return val;
}

intmax_t strtoimax(const char * restrict nptr, char ** restrict end, int base)
{
	int sgn;
	uintmax_t val;
	val = _strtoumax_sgn(nptr, end, base, &sgn);
	if (val > INTMAX_MAX) {
		if (sgn < 0) {
			return INTMAX_MIN;
		}
		return INTMAX_MAX;
	}
	if (sgn < 0) {
		return -(intmax_t)val;
	}
	return val;
}
