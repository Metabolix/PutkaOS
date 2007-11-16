#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <pos/syscalls.h>

double atof(const char *str)
{
	return strtod(str, 0);
}
int atoi(const char *str)
{
	return (int) strtol(str, 0, 10);
}
long int atol(const char *str)
{
	return strtol(str, 0, 10);
}
long long int atoll(const char *str)
{
	return strtoll(str, 0, 10);
}

double strtod(const char * restrict nptr, char ** restrict end)
{
	return strtold(nptr, end);
}
float strtof(const char * restrict nptr, char ** restrict end)
{
	return strtold(nptr, end);
}
long double strtold(const char * restrict nptr, char ** restrict end) // TODO
{
/*
The expected form of the subject sequence is an optional plus or minus sign, then one of
the following:
— a nonempty sequence of decimal digits optionally containing a decimal-point
    character, then an optional exponent part as deﬁned in 6.4.4.2;
— a 0x or 0X, then a nonempty sequence of hexadecimal digits optionally containing a
    decimal-point character, then an optional binary exponent part as deﬁned in 6.4.4.2;
— INF or INFINITY, ignoring case
— NAN or NAN(n-char-sequenceopt), ignoring case in the NAN part, where:
           n-char-sequence:
                    digit
                    nondigit
                    n-char-sequence digit
                    n-char-sequence nondigit
*/
	int i = 0, j = 0;
	i /= j;
	return 0;
}

long int strtol(const char * restrict nptr, char ** restrict end, int base)
{
	intmax_t val;
	val = strtoimax(nptr, end, base);
	if (val > LONG_MAX) {
		return LONG_MAX;
	}
	if (val < LONG_MIN) {
		return LONG_MAX;
	}
	return val;
}
unsigned long int strtoul(const char * restrict nptr, char ** restrict end, int base)
{
	uintmax_t val;
	val = strtoumax(nptr, end, base);
	if (val > ULONG_MAX) {
		return ULONG_MAX;
	}
	return val;
}
long long int strtoll(const char * restrict nptr, char ** restrict end, int base)
{
	intmax_t val;
	val = strtoimax(nptr, end, base);
	if (val > LLONG_MAX) {
		return LONG_MAX;
	}
	if (val < LLONG_MIN) {
		return LONG_MAX;
	}
	return val;
}
unsigned long long int strtoull(const char * restrict nptr, char ** restrict end, int base)
{
	uintmax_t val;
	val = strtoumax(nptr, end, base);
	if (val > ULLONG_MAX) {
		return ULLONG_MAX;
	}
	return val;
}

static unsigned int _rand_i1 = 1, _rand_i2 = 1;
int rand(void)
{
	_rand_i1 -= 0x012357bf;
	_rand_i2 -= 0xfedca201;
	_rand_i1 += (_rand_i1 >> 16) ^ (_rand_i2 << 16);
	_rand_i2 -= (_rand_i2 << 16) ^ (_rand_i2 >> 16);
	return (_rand_i1 ^ _rand_i2) & RAND_MAX;
}
void srand(unsigned int seed)
{
	_rand_i1 = _rand_i2 = seed;
}

void *calloc(size_t nmemb, size_t size)
{
	unsigned long long s = nmemb;
	s *= size;
	if (s > 0xffffffff) {
		return 0;
	}
	void *ptr = malloc(s);
	if (!ptr) {
		return 0;
	}
	memset(ptr, 0, s);
	return ptr;
}

void free(void *ptr)
{
	syscall_free(ptr);
}

void *malloc(size_t size)
{
	return syscall_malloc(size);
}
void *realloc(void *ptr, size_t size)
{
	return syscall_realloc(ptr, size);
}

// TODO: abort, atexit, exit, _Exit
// TODO: getenv, system

// TODO: bsearch, qsort

int abs(int i)
{
	return (i < 0 ? -i : i);
}
long int labs(long int i)
{
	return (i < 0 ? -i : i);
}
long long int llabs(long long int i)
{
	return (i < 0 ? -i : i);
}

div_t div(int yla, int ala)
{
	div_t ret = {
		.quot = yla / ala,
		.rem = yla % ala,
	};
	return ret;
}
ldiv_t ldiv(long int yla, long int ala)
{
	ldiv_t ret = {
		.quot = yla / ala,
		.rem = yla % ala,
	};
	return ret;
}

// TODO: lldiv
/*lldiv_t lldiv(long long int yla, long long int ala)
{
	lldiv_t ret;
	ret.quot = int64_div_rem(yla, ala, &ret.rem);
	return ret;
}*/

// TODO: mblen, mbtowc, wctomb, mbstowcs, wcstombs
