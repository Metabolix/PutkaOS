#ifndef _ISR_H
#define _ISR_H 1

#include <stdint.h>

struct isr_regs {
	uint32_t gs, fs, es, ds, ss;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, error_code;
	uint32_t eip, cs, eflags;
};
typedef int (*isr_handler_t)(struct isr_regs *regs);

extern void dump_isr_regs(const struct isr_regs *regs);

extern void isr_install(void);

#endif
