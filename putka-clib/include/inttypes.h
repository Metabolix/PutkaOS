#ifndef _INTTYPES_H
#define _INTTYPES_H 1

#include <stdint.h>

typedef struct _imaxdiv_t {
	intmax_t quot, rem;
} imaxdiv_t;

extern intmax_t imaxabs(intmax_t j);
extern imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
extern uintmax_t strtoumax(const char * restrict nptr, char ** restrict end, int base);
extern intmax_t strtoimax(const char * restrict nptr, char ** restrict end, int base);

// TODO: fprintf / fscanf macros

// TODO: wcstoimax, wcstoumax

#endif
