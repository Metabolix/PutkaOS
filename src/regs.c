#include <regs.h>
#include <screen.h>
#include <ctype.h>

void dump_regs(const struct regs *regs)
{
	print("Registers:\n");
	kprintf(" eax = %08x     ebx = %08x     ecx = %08x     edx = %08x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
	kprintf(" esp = %08x     ebp = %08x      ss = %08x\n", regs->esp, regs->ebp, regs->ss);
	kprintf(" eip = %08x      cs = %08x  eflags = %08x\n", regs->eip, regs->cs, regs->eflags);
	kprintf(" esi = %08x     edi = %08x\n", regs->esi, regs->edi);
	kprintf("  ds = %08x      es = %08x      fs = %08x      gs = %08x\n", regs->ds, regs->es, regs->fs, regs->gs);

	print("\nStack:\n");
	int i, j;
	typedef struct _c16_t {char c[16]; } c16_t;
	const c16_t * const stack = (const void*)(regs + 1);

	for (i = 0; i < 8; ++i) {
		c16_t c = stack[i];
		char buf[32], *ptr = buf;
		for (j = 0; j < 16; ++j) {
			if (j % 8 == 0) {
				*ptr++ = ' ';
				*ptr++ = ' ';
			}
			if (!isprint(c.c[j])) {
				*ptr++ = '.';
			} else {
				*ptr++ = c.c[j];
			}
		}
		*ptr++ = 0;
		kprintf(" %08x %08x  %08x %08x  | %s\n", c, buf);
	}
	putch('\n');
}
