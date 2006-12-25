#include <regs.h>
#include <screen.h>

void dump_regs(const struct regs_t *regs)
{
	print("Registers:\n");
	kprintf(" eax = %08x     ebx = %08x     ecx = %08x     edx = %08x\n",
	    regs->eax,     regs->ebx,     regs->ecx,     regs->edx);
	kprintf(" esp = %08x     ebp = %08x      ss = %08x\n",
	    regs->esp,     regs->ebp,      regs->ss);
	kprintf(" eip = %08x      cs = %08x  eflags = %08x\n",
	    regs->eip,      regs->cs,  regs->eflags);
	kprintf(" esi = %08x     edi = %08x\n",
	    regs->esi,     regs->edi);
	kprintf("  ds = %08x      es = %08x      fs = %08x      gs = %08x\n",
	     regs->ds,      regs->es,      regs->fs,      regs->gs);
}
