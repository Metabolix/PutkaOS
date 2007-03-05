#ifndef _STDINT_H
#define _STDINT_H 1

typedef unsigned int uint_t;

/* Exact width types */
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef signed long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

/* Minimum-width types */
typedef signed char int_least8_t;
typedef signed short int_least16_t;
typedef signed long int_least32_t;
typedef signed long long int_least64_t;

typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned long uint_least32_t;
typedef unsigned long long uint_least64_t;

/* Fastest types... TODO: test & check */
typedef signed char int_fast8_t;
typedef signed short int_fast16_t;
typedef signed long int_fast32_t;
typedef signed long long int_fast64_t;

typedef unsigned char uint_fast8_t;
typedef unsigned short uint_fast16_t;
typedef unsigned long uint_fast32_t;
typedef unsigned long long uint_fast64_t;

/* void* <-> (u)intptr_t */
typedef int intptr_t;
typedef unsigned int uintptr_t;

/* Greatest-width types */
typedef int64_t intmax_t;
typedef uint64_t uintmax_t;

/* Sizes (TODO)*/
#define UINT32_MAX (0xffffffffUL)
#define INT32_MAX (0x7fffffffL)
#define INT32_MIN (-0x7fffffffL-1L)

#endif
