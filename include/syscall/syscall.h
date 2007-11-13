#ifndef _SYSCALL_H
#define _SYSCALL_H 1

#include <stdint.h>

typedef intptr_t (*syscall_t)(uint_t eax, intptr_t ecx, intptr_t edx);
extern intptr_t make_syscall(uint_t eax, intptr_t ecx, intptr_t edx);

#endif
