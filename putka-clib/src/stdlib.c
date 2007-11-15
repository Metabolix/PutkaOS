#include <stdlib.h>
#include <string.h>
#include <sys/syscalls.h>

// TODO: atof, atoi, atol, atoll
// TODO: strtod, strtof, strtold
// TODO: strtol, strtoll, strtoul, strtoull

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
