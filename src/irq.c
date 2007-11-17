#include <kprintf.h>
#include <irq.h>
#include <idt.h>
#include <tss.h>
#include <io.h>
#include <panic.h>
#include <stdint.h>
#include <misc_asm.h>

#define io_wait() asm_nop()

irq_handler_t irq_handlers[16];
volatile char irq_wait[16] = {0};
int irq_handling = -1;

void irq_illegal_handler(void)
{
	panic("Illegal IRQ! (Impossible?)");
}

void irq_null_handler(void)
{
	kprintf("IRQ %d without handler!\n", irq_handling);
}

int is_in_irq_handler(void)
{
	return irq_handling >= 0;
}

void irq_unmask(void)
{
	outportb(0x21, 0);
	outportb(0xa1, 0);
	asm_sti();
	asm_hlt();
}

void irq_remap(uint8_t offset1, uint8_t offset2)
{
	outportb(0x20, 0x11); io_wait();
	outportb(0x21, offset1); io_wait();
	outportb(0x21, 4); io_wait();
	outportb(0x21, 0x01); io_wait();

	outportb(0xA0, 0x11); io_wait();
	outportb(0xA1, offset2); io_wait();
	outportb(0xA1, 2); io_wait();
	outportb(0xA1, 0x01); io_wait();
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

void irq_install(void)
{
	extern void irq0x00();
	extern void irq0x01();
	extern void irq0x02();
	extern void irq0x03();
	extern void irq0x04();
	extern void irq0x05();
	extern void irq0x06();
	extern void irq0x07();
	extern void irq0x08();
	extern void irq0x09();
	extern void irq0x0a();
	extern void irq0x0b();
	extern void irq0x0c();
	extern void irq0x0d();
	extern void irq0x0e();
	extern void irq0x0f();

	int i;
	for (i = 0; i < 16; ++i) {
		irq_handlers[i] = irq_null_handler;
	}

	irq_remap(0x20, 0x28);

	idt_set_interrupt(0x20, irq0x00, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x21, irq0x01, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x22, irq0x02, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x23, irq0x03, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x24, irq0x04, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x25, irq0x05, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x26, irq0x06, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x27, irq0x07, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x28, irq0x08, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x29, irq0x09, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x2A, irq0x0a, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x2B, irq0x0b, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x2C, irq0x0c, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x2D, irq0x0d, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x2E, irq0x0e, IDT_PRIV_KERNEL);
	idt_set_interrupt(0x2F, irq0x0f, IDT_PRIV_KERNEL);
}

void install_irq_handler(unsigned int irq, irq_handler_t handler)
{
	if (irq >= 16) {
		kprintf("Trying to install irq handler %i, out of indexes!\n", irq);
		panic("uninstall_irq_handler: illegal irq!");
	}
	if (irq_handlers[irq] != irq_null_handler) {
		kprintf("Trying to install irq handler number %u but it exists!\n", irq);
		panic("uninstall_irq_handler: illegal irq!");
	} else {
		irq_handlers[irq] = handler;
	}
}

void uninstall_irq_handler(unsigned int irq)
{
	if (irq >= 16) {
		kprintf("Trying to uninstall irq handler %i, out of indexes!\n", irq);
		panic("uninstall_irq_handler: illegal irq!");
	}
	if (irq_handlers[irq] == irq_null_handler) {
		kprintf("Trying to uninstall irq handler number %u but it doesn't exist!\n", irq);
		panic("uninstall_irq_handler: illegal irq!");
	} else {
		irq_handlers[irq] = irq_null_handler;
	}
}
