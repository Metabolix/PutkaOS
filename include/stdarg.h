#ifndef _STDARG_H
#define _STDARG_H

typedef struct va_list_s {
	void* va_ptr;
} va_list;

#define va_start(ap, argN) {ap.va_ptr = &argN + 1;}
#define va_copy(dest, src) {dest.va_ptr = src.va_ptr;}
#define va_arg(ap, type) (*(type*)ap.va_ptr++)
#define va_end(ap); {ap.va_ptr = 0;}

#endif
