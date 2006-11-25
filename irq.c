#include <screen.h>
#include <idt.h>
#include <io.h>
#include <panic.h>

#define io_wait() asm("nop")

void * irq_handlers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

void install_irq()
{
	irq_remap(0x20, 0x28);
	outportb(0x20, 0x20);
}	

void install_irq_handler(int irq, void (*irqhandler()))  {
	if(irq < 16 && irq >= 0)
		irq_handlers[irq] = irqhandler;
	else
		print("Trying to install irq handler on irq, which doesn't exist!\n");
}

void uninstall_irq_handler(int irq) {
	if(irq < 16 && irq >= 0) {
		if(irq_handlers[irq] == 0) {
			print("Trying to uninstall irq handler number ");
			/*print_number((unsigned int) irq);*/
			putch('0' + ((irq & 90)));
			putch('0' + (irq & 10));
			print(" but it doesn't exist\n");
		} else {
			irq_handlers[irq] = 0;
		}
	} else {
		print("Trying to uninstall irq handler, out of indexes\n");
	}
}

void irq_handler(unsigned int irq) /* NOTICE: This should be called only from our assembly code! */
{
	void (*handler)();

	if(irq != 0) {
		print("Got interrupt on ");
		print_hex(irq);
		print("\n");
	}

	if(irq < 16 && irq >= 0) {
		if(irq_handlers[irq]) {
			handler = irq_handlers[irq];
			handler();
		} else {
			print ("Got interrupt, but we don't have handler for it!\n");
		}
	} else {
		panic("Irq_handler got irq which doesn't exist\n");
	}
	if(irq >= 8)
		outportb(0xA0, 0x20);
	outportb(0x20, 0x20);
}
