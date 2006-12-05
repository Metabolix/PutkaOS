#include <screen.h>
#include <idt.h>
#include <io.h>
#include <panic.h>

#define io_wait() asm("nop")

void * irq_handlers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char irq_wait[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


void irq_remap(int offset1, int offset2)
{
        /*char  a1, a2;

        a1=inportb(0x21); */  /* save masks */
        /*a2=inportb(0xA1);*/

        outportb(0x20, 0x11);  /* starts the initialization sequence */
        io_wait();
        outportb(0xA0, 0x11);
        io_wait();
        outportb(0x21, offset1);
	io_wait();
        outportb(0xA1, offset2);
        io_wait();
        outportb(0x21, 2);
        io_wait();
	outportb(0xA1, 4);
        io_wait();

        outportb(0x21, 0x05);
        io_wait();
        outportb(0xA1, 0x01);
        io_wait();

        /*outportb(0x21, a1);*/   /* restore saved masks */
        /*outportb(0xA1, a2);*/
	print("IRQs remapped\n");
}

int get_irq_wait(int irq);
__asm__(
"get_irq_wait:\n"
"    movl 4(%esp), %eax\n"
"    movl irq_wait(,%eax,4), %eax\n"
"    ret\n"
);

void wait_irq(int irq)
{
	if(irq < 16 && irq >= 0) {
		/*kprintf("wait_irq: Waiting for irq %u (%#x)\n", irq, irq);*/
		while (get_irq_wait(irq));
		/*kprintf("wait_irq: Got irq %u (%#x)\n", irq, irq);*/
	} else {
		kprintf("wait_irq: Invalid irq: %u (%#x)\n", irq, irq);
	}
}

void prepare_wait_irq(int irq)
{
	if(irq < 16 && irq >= 0) {
		irq_wait[irq] = 1;
	}
}

void install_irq()
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

	outportb(0x20, 0x20);
}

void install_irq_handler(int irq, void (*irqhandler()))
{
	if(irq < 16 && irq >= 0) {
		irq_handlers[irq] = irqhandler;
	} else {
		kprintf("Trying to install irq handler on irq %i, which doesn't exist!\n", irq);
	}
}

void uninstall_irq_handler(int irq)
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

void irq_handler(unsigned int irq) /* NOTICE: This should be called only from our assembly code! */
{
	void (*handler)();


	if(irq < 16 && irq >= 0) {
		if(irq_handlers[irq]) {
			handler = irq_handlers[irq];
			handler();
		} else {
			kprintf("Got interrupt on %u, but we don't have handler for it!\n", irq);
		}
	} else {
		kprintf("Irq_handler got irq %u which doesn't exist\n", irq);
	}
	if(irq >= 8) {
		outportb(0xA0, 0x20);
	}
	outportb(0x20, 0x20);
	if(irq_wait[irq]) {
		irq_wait[irq] = 0;
	}
}
