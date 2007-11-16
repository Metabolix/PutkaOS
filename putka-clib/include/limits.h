#ifndef _LIMITS_H
#define _LIMITS_H 1

#define CHAR_BIT (8)
#define SCHAR_MIN ((signed char) - (1 << 7))
#define SCHAR_MAX ((signed char) + ((1 << 7) - 1))
#define UCHAR_MAX ((unsigned char) + ((1 << CHAR_BIT) - 1))
#define CHAR_MIN (((int)((char)SCHAR_MIN) == (int)(SCHAR_MIN)) ? (char)SCHAR_MIN : (char)0)
#define CHAR_MAX (((int)((char)UCHAR_MAX) == (int)(UCHAR_MAX)) ? (char)UCHAR_MAX : (char)SCHAR_MAX)

// TODO: MB_LEN_MAX

#define SHRT_MIN ((signed short int) - (1ULL << 15))
#define SHRT_MAX ((signed short int) + ((1ULL << 15) - 1))
#define USHRT_MAX ((unsigned short int) + ((1ULL << 16) - 1))

#define INT_MIN ((signed int) - (1ULL << 31))
#define INT_MAX ((signed int) + ((1ULL << 31) - 1))
#define UINT_MAX ((unsigned int) + ((1ULL << 32) - 1))

#define LONG_MIN ((signed long int) - (1ULL << 31))
#define LONG_MAX ((signed long int) + ((1ULL << 31) - 1))
#define ULONG_MAX ((unsigned long int) + ((1ULL << 32) - 1))

#define LLONG_MIN ((signed long long int) - 0x7fffffffffffffff - 1)
#define LLONG_MAX ((signed long long int) + 0x7fffffffffffffff)
#define ULLONG_MAX ((unsigned long long int) + 0xffffffffffffffff)

#endif
