#include <screen.h>
#include <irq.h>
#include <idt.h>
#include <io.h>
#include <panic.h>
#include <thread.h>
#include <regs.h>
#include <misc_asm.h>

#define io_wait() asm_nop()

irq_handler_t irq_handlers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile char irq_wait[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char irq_handling = 0;

void irq_remap(unsigned char offset1, unsigned char offset2)
{
	/*char  a1, a2;

	a1=inportb(0x21); */  /* save masks */
	/*a2=inportb(0xA1);*/

	/* starts the initialization sequence */
	outportb(0x20, 0x11); io_wait();
	outportb(0x21, offset1); io_wait();
	outportb(0x21, 4); io_wait();
	outportb(0x21, 0x01); io_wait();

	outportb(0xA0, 0x11); io_wait();
	outportb(0xA1, offset2); io_wait();
	outportb(0xA1, 2); io_wait();
	outportb(0xA1, 0x01); io_wait();

	/*outportb(0x21, a1);*/   /* restore saved masks */
	/*outportb(0xA1, a2);*/
	print("IRQs remapped\n");
}

#define asm_wait_irq(x) asm_hlt_until_true((char*)&irq_wait[x])

void wait_irq(unsigned int irq)
{
	if (irq < 16) {
		asm_wait_irq(irq);
	} else {
		kprintf("wait_irq: Invalid irq: %u (%#x)\n", irq, irq);
	}
}

void prepare_wait_irq(unsigned int irq)
{
	if (irq < 16 && irq >= 0) {
		irq_wait[irq] = 1;
	}
}

unsigned char in_irq_handler(void)
{
	return irq_handling;
}

void irq_install(void)
{
	extern void irq0();
	extern void irq1();
	extern void irq2();
	extern void irq3();
	extern void irq4();
	extern void irq5();
	extern void irq6();
	extern void irq7();
	extern void irq8();
	extern void irq9();
	extern void irq10();
	extern void irq11();
	extern void irq12();
	extern void irq13();
	extern void irq14();
	extern void irq15();

	irq_remap(0x20, 0x28);

	idt_set_gate(0x20, (unsigned)irq0, 0x08, 0x8E);
	idt_set_gate(0x21, (unsigned)irq1, 0x08, 0x8E);
	idt_set_gate(0x22, (unsigned)irq2, 0x08, 0x8E);
	idt_set_gate(0x23, (unsigned)irq3, 0x08, 0x8E);
	idt_set_gate(0x24, (unsigned)irq4, 0x08, 0x8E);
	idt_set_gate(0x25, (unsigned)irq5, 0x08, 0x8E);
	idt_set_gate(0x26, (unsigned)irq6, 0x08, 0x8E);
	idt_set_gate(0x27, (unsigned)irq7, 0x08, 0x8E);
	idt_set_gate(0x28, (unsigned)irq8, 0x08, 0x8E);
	idt_set_gate(0x29, (unsigned)irq9, 0x08, 0x8E);
	idt_set_gate(0x2A, (unsigned)irq10, 0x08, 0x8E);
	idt_set_gate(0x2B, (unsigned)irq11, 0x08, 0x8E);
	idt_set_gate(0x2C, (unsigned)irq12, 0x08, 0x8E);
	idt_set_gate(0x2D, (unsigned)irq13, 0x08, 0x8E);
	idt_set_gate(0x2E, (unsigned)irq14, 0x08, 0x8E);
	idt_set_gate(0x2F, (unsigned)irq15, 0x08, 0x8E);
}

void install_irq_handler(unsigned int irq, irq_handler_t handler)
{
	if (irq < 16) {
		irq_handlers[irq] = handler;
	} else {
		kprintf("Trying to install irq handler on irq %i, which doesn't exist!\n", irq);
	}
}

void uninstall_irq_handler(unsigned int irq)
{
	if(irq < 16 && irq >= 0) {
		if(irq_handlers[irq] == 0) {
			kprintf("Trying to uninstall irq handler number %u but it doesn't exist\n", irq);
		} else {
			irq_handlers[irq] = 0;
		}
	} else {
		kprintf("Trying to uninstall irq handler %i, out of indexes\n", irq);
	}
}

void irq_handler(struct regs_t *regs) /* NOTICE: This should be called only from our assembly code! */
{
	irq_handling = 1;
	if (regs->int_no & 0xfffffff0) {
		kprintf("Irq_handler got irq %u which doesn't exist\n", regs->int_no);
		dump_regs(regs);
	} else {
		if (irq_handlers[regs->int_no]) {
			irq_handlers[regs->int_no](regs);
		} else {
			/*kprintf("Got interrupt on %u, but we don't have handler for it!\n", regs->int_no);*/
		}
		if (regs->int_no >= 8) {
			outportb(0xA0, 0x20);
		}
		outportb(0x20, 0x20);
		irq_wait[regs->int_no] = 0;
	}
	next_thread();
	irq_handling = 0;
}
