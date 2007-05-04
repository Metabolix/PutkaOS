#ifndef _STDARG_H
#define _STDARG_H

typedef struct va_list_s {
	void* va_ptr;
} va_list;

#define va_start(ap, argN) ((ap.va_ptr = &argN + 1),(void)0)
#define va_copy(dest, src) ((dest.va_ptr = src.va_ptr),(void)0)
#define va_arg(ap, type) ((ap.va_ptr = ((int*)ap.va_ptr + ((sizeof(type)+sizeof(int)-1)/sizeof(int)))), *(type*)((int*)ap.va_ptr - ((sizeof(type)+sizeof(int)-1)/sizeof(int))))
#define va_end(ap); ((ap.va_ptr = 0),(void)0)

#endif
