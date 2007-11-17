#include <isr.h>
#include <idt.h>
#include <gdt.h>
#include <panic.h>
#include <kprintf.h>
#include <ctype.h>
#include <multitasking/multitasking.h>
#include <misc_asm.h>
#include <memory/pagefault.h>

struct isr_t {
	const char *name;
	isr_handler_t handler;
};

int isr_panic_handler(struct isr_regs *regs);
int isr_null_handler(struct isr_regs *regs);

/* These are defined in our isrs.asm */
extern void isr0x00();
extern void isr0x01();
extern void isr0x02();
extern void isr0x03();
extern void isr0x04();
extern void isr0x05();
extern void isr0x06();
extern void isr0x07();
//extern void isr0x08(); // GPF
extern void isr0x09();
extern void isr0x0a();
extern void isr0x0b();
extern void isr0x0c();
extern void isr0x0d();
//extern void isr0x0e(); // Page fault
extern void isr0x0f();
extern void isr0x10();
extern void isr0x11();
extern void isr0x12();
extern void isr0x13();
extern void isr0x14();
extern void isr0x15();
extern void isr0x16();
extern void isr0x17();
extern void isr0x18();
extern void isr0x19();
extern void isr0x1a();
extern void isr0x1b();
extern void isr0x1c();
extern void isr0x1d();
extern void isr0x1e();
extern void isr0x1f();

/* Erikoisempia tapauksia */
extern void isr_scheduler();
extern void isr_thread_ending();

void isr_install(void)
{
	idt_set_interrupt(0x00, isr0x00, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x01, isr0x01, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x02, isr0x02, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x03, isr0x03, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x04, isr0x04, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x05, isr0x05, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x06, isr0x06, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x07, isr0x07, IDT_PRIV_KERNEL);

	idt_set_task_gate(0x08, 8 * GDT_DOUBLE_FAULT_TSS, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x09, isr0x09, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x0a, isr0x0a, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x0b, isr0x0b, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x0c, isr0x0c, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x0d, isr0x0d, IDT_PRIV_KERNEL);
	idt_set_task_gate(0x0e, 8 * GDT_PAGE_FAULT_TSS, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x0f, isr0x0f, IDT_PRIV_KERNEL);

	idt_set_interrupt(0x10, isr0x10, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x11, isr0x11, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x12, isr0x12, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x13, isr0x13, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x14, isr0x14, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x15, isr0x15, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x16, isr0x16, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x17, isr0x17, IDT_PRIV_KERNEL);

	idt_set_interrupt(0x18, isr0x18, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x19, isr0x19, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x1a, isr0x1a, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x1b, isr0x1b, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x1c, isr0x1c, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x1d, isr0x1d, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x1e, isr0x1e, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x1f, isr0x1f, IDT_PRIV_KERNEL);

	idt_set_interrupt(IDT_THREAD_ENDING, isr_thread_ending, IDT_PRIV_USER);
	idt_set_interrupt(IDT_SCHEDULER, isr_scheduler, IDT_PRIV_USER);
}
struct isr_t isrs[32] = {
	{"Division By Zero", 0},
	{"Debug", 0},
	{"Non Maskable Interrupt", 0},
	{"Breakpoint", 0},
	{"Into Detected Overflow", 0},
	{"Out of Bounds", 0},
	{"Invalid Opcode", 0},
	{"No Coprocessor", 0},

	/* Task gate */ {"Double Fault", 0},
	{"Coprocessor Segment Overrun", 0},
	{"Bad TSS", 0},
	{"Segment Not Present", 0},
	{"Stack Fault", 0},
	{"General Protection Fault", 0},
	/* Task gate */ {"Page Fault", 0},
	{"Unknown Interrupt", 0},

	{"Coprocessor Fault", 0},
	{"Alignment Check", 0},
	{"Machine Check", 0},
	{"Streaming SIMD Extensions -fault..?", 0},
	{"Reserved", 0},
	{"Reserved", 0},
	{"Reserved", 0},
	{"Reserved", 0},

	{"Reserved", 0},
	{"Reserved", 0},
	{"Reserved", 0},
	{"Reserved", 0},
	{"Reserved", 0},
	{"Reserved", 0},
	{"Reserved", 0},
	{"Reserved", 0}
};

void dump_isr_regs(const struct isr_regs *regs)
{
	kprintf("Registers:\n");
	kprintf(" eax = %08x     ebx = %08x     ecx = %08x     edx = %08x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
	kprintf(" esp = %08x     ebp = %08x      ss = %08x\n", regs->esp, regs->ebp, regs->ss);
	kprintf(" eip = %08x      cs = %08x  eflags = %08x\n", regs->eip, regs->cs, regs->eflags);
	kprintf(" esi = %08x     edi = %08x\n", regs->esi, regs->edi);
	kprintf("  ds = %08x      es = %08x      fs = %08x      gs = %08x\n", regs->ds, regs->es, regs->fs, regs->gs);
#if 0
	kprintf("\nStack:\n");
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
#endif
	kprintf("\n");
}

void isr_handler(struct isr_regs *regs)
{
	int r;
	if (regs->int_no > 31) { // > 31
		kprintf("ISR handler: illegal interrupt (%d)!\n", regs->int_no);
		r = -1;
	} else if (isrs[regs->int_no].handler) {
		r = isrs[regs->int_no].handler(regs);
	} else {
		r = isr_panic_handler(regs);
	}
	if (r) {
		kill_thread(active_tid);
	}
}

int isr_null_handler(struct isr_regs *regs)
{
	kprintf("(ISR number %d)\n", regs->int_no);
	return 0;
}

int isr_panic_handler(struct isr_regs *regs)
{
	if (regs->int_no > 7) { // > 7
		kprintf("Problem: %s, code %i\n", isrs[regs->int_no].name, regs->error_code);
	} else {
		kprintf("Problem: %s\n", isrs[regs->int_no].name);
	}
	dump_isr_regs(regs);
	kprintf("Killing thread %i, process %i\n", active_tid, active_pid);
	return -1;
}
