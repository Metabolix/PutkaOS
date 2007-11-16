#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <stddef.h>

typedef struct _div_t {
	int quot, rem;
} div_t;
typedef struct _ldiv_t {
	long int quot, rem;
} ldiv_t;
typedef struct _lldiv_t {
	long long int quot, rem;
} lldiv_t;

#define EXIT_FAILURE (-1)
#define EXIT_SUCCESS (0)

#define RAND_MAX (0x7fffffff)

// TODO: MB_CUR_MAX

extern double atof(const char *str);
extern int atoi(const char *str);
extern long int atol(const char *str);
extern long long int atoll(const char *str);

extern double strtod(const char * restrict nptr, char ** restrict end);
extern float strtof(const char * restrict nptr, char ** restrict end);
extern long double strtold(const char * restrict nptr, char ** restrict end);

extern long int strtol(const char * restrict nptr, char ** restrict end, int base);
extern unsigned long int strtoul(const char * restrict nptr, char ** restrict end, int base);
extern long long int strtoll(const char * restrict nptr, char ** restrict end, int base);
extern unsigned long long int strtoull(const char * restrict nptr, char ** restrict end, int base);

int rand(void);
void srand(unsigned int seed);

extern void *calloc(size_t nmemb, size_t size);
extern void free(void *ptr);
extern void *malloc(size_t size);
extern void *realloc(void *ptr, size_t size);

// TODO: abort, atexit, exit, _Exit
// TODO: getenv, system

// TODO: bsearch, qsort

int abs(int i);
long int labs(long int i);
long long int llabs(long long int i);
div_t div(int yla, int ala);
ldiv_t ldiv(long int yla, long int ala);
// lldiv_t lldiv(long long int yla, long long int ala) // TODO: lldiv

// TODO: mblen, mbtowc, wctomb, mbstowcs, wcstombs

#endif
