#ifndef _REGS_H
#define _REGS_H 1
struct regs_t {
	unsigned int gs, fs, es, ds;
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned int int_no, error_code;
	unsigned int eip, cs, eflags, useresp, ss;
};
extern void dump_regs(const struct regs_t *regs);
#endif
