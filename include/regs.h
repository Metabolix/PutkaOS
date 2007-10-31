#ifndef _REGS_H
#define _REGS_H 1

#include <stdint.h>

struct regs {
	uint32_t gs, fs, es, ds, ss;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, error_code;
	uint32_t eip, cs, eflags;
};

extern void dump_regs(const struct regs *regs);

#endif
