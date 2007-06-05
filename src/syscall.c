#include <idt.h>
#include <regs.h>
#include <screen.h>

void call_syscall(struct regs_t regs)
{
	switch(regs.eax) {
		case 0:
			print("syscall number 0!\n");
			break;
		case 1:
			kprintf("syscall 1 with 2 params: %d and %d\n", regs.ebx, regs.ecx);
			break;
		default:
			kprintf("undefined syscall number %d\n", regs.eax);
	}
}

void syscall(void);
__asm__("syscall:\n" /* TODO: Change stack? */
	"pusha\n"
	"push %ss\n"
	"push %ds\n"
	"push %es\n"
	"push %fs\n"
	"push %gs\n"
	"call call_syscall\n"
	"pop %gs\n"
	"pop %fs\n"
	"pop %es\n"
	"pop %ds\n"
	"pop %ss\n"
	"popa\n"
	"iret\n"
);


void init_syscalls(void)
{
	idt_set_gate(0x80, (unsigned)syscall, 0x08, 0x8E);
}
