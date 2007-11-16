#ifndef _SYSCALLS_H
#define _SYSCALLS_H 1

#include <stdint.h>

#define SYSCALL_TYPEDEFS 1

#define SYSCALL_MACRO(num, func, name, proto, ...) \
extern proto;
#include <pos/syscalls.list.h>
#undef SYSCALL_MACRO

#endif
