#include <idt.h>
#include <panic.h>
#include <screen.h>
#include <regs.h>
#include <thread.h>

struct isr_t {
	const char *name;
	void (*handler)(struct regs_t *regs);
};
void page_fault_handler(struct regs_t *regs);

extern void isr0(); /* These are defined in our isrs.asm */
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

void isrs_install(void)
{
	idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
	idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
	idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
	idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
	idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
	idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
	idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
	idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);

	idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
	idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
	idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
	idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
	idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
	idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
	idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
	idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);

	idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
	idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
	idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
	idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
	idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
	idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
	idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
	idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);

	idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
	idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
	idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
	idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
	idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
	idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
	idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
	idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
	print("ISRs enabled\n");
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

	{"Double Fault", 0},
	{"Coprocessor Segment Overrun", 0},
	{"Bad TSS", 0},
	{"Segment Not Present", 0},
	{"Stack Fault", 0},
	{"General Protection Fault", 0},
	{"Page Fault", page_fault_handler},
	{"Unknown Interrupt", 0},

	{"Coprocessor Fault", 0},
	{"Alignment Check", 0},
	{"Machine Check", 0},
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
	{"Reserved", 0},
	{"Reserved", 0}
};

void* get_cr2(void);
__asm__(
"get_cr2:\n"
"    movl %cr2, %eax\n"
"    ret\n"
);

void page_fault_handler(struct regs_t *regs)
{
	void* cr2 = get_cr2();
	kprintf("Page Fault!\nThread %i, process %i\n", active_thread, active_process);
	dump_regs(regs);

	kprintf("Trying to %s address %p, %s. Processor is in %s mode.\n",
		((regs->error_code & 2) ? "write" : "read"),
		cr2,
		((regs->error_code & 1) ? "page is not present" : "page-level protection violation"),
		((regs->error_code & 4) ? "user" : "supervisor"));
	panic("Can't survive a page fault!");
}

void isr_handler(struct regs_t *regs)
{
	if (regs->int_no & 0xffffffe0) { // > 31
		panic("isr_handler got an absurd interrupt!");
	}
	if (isrs[regs->int_no].handler) {
		isrs[regs->int_no].handler(regs);
	} else {
		if (regs->int_no & 0xfffffff8) { // > 7
			kprintf("%s, code %i\n", isrs[regs->int_no], regs->error_code);
		} else {
			kprintf("%s\n", isrs[regs->int_no]);
		}
		kprintf("Thread %i, process %i\n", active_thread, active_process);
		dump_regs(regs);
		panic("ISR not handled!");
	}
}
