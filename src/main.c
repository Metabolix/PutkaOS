#include <screen.h>
#include <gdt.h>
#include <irq.h>
#include <keyboard.h>
#include <memory.h>
#include <idt.h>
#include <gdt.h>
#include <isrs.h>
#include <io.h>
#include <panic.h>
#include <malloc.h>
#include <timer.h>
#include <floppy.h>
#include <multiboot.h>
#include <regs.h>
#include <thread.h>

const char systeemi[] = "PutkaOS";
const char versio[] = "v0.001";
BD_DESC *dev;
char buf[512];
int i;

void testattava_koodi()
{
#ifdef TUHOA_KORPUN_SISALTO
	dev = dopen(&fd_devices[0]);

	for(i = 0; i < 512; i++)
		buf[i] = 'P';
	for(i = 0; i < 2880; i++) {
		dwrite(buf, 512, 1, dev);
		kprintf("Block %d\n", i);
	}
	dclose(dev);
	i = 0;
#endif
}

void kmain(multiboot_info_t* mbt, unsigned int magic)
{
	cls();
	if ((mbt->flags & 1) == 0) {
		panic("Mbt->flags bit 1 wasn't 1\n");
	}
	gdt_install();
	idt_install();
	isrs_install();
	init_memory(mbt->mem_upper + mbt->mem_lower);
	irq_install();
	keyboard_install();
	timer_install();
	install_floppy();
	malloc_init();

	outportb(0x21,0x0); /* Don't mask any IRQ */
	outportb(0xa1,0x0);
	asm __volatile__("sti");	/* Allow interrupts */

	reset_floppy();

	testattava_koodi();

	kprintf("%s %s is up and running _o/\n", systeemi, versio);
	start_threading();
	//for (;;);
}
