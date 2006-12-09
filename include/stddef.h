#ifndef _STDDEF_H
#define _STDDEF_H

typedef int ptrdiff_t;
typedef unsigned int size_t;
typedef unsigned int wchar_t;

#define NULL 0
#define offsetof(type, member) ((int)&(((type*)0)->member))

#endif
