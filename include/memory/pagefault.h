#ifndef _PAGEFAULT_H
#define _PAGEFAULT_H 1

#include <regs.h>

extern void page_fault_handler(struct regs *regs);

#endif
