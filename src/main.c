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

void kmain(multiboot_info_t* mbt, unsigned int magic)
{
	cls();
	if((mbt->flags & 1) == 0)
		panic("Mbt->flags bit 1 wasn't 1\n");
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

#if 0
	dev = dopen(&fd_devices[0]);
	dread(buf, 1, 256, dev);
	for (i = 0; i < 128; ++i) {
		kprintf("%02x ", (int)(unsigned char)buf[i]);
		if ((i+1)%16 == 0) kprintf("\n");
	}
	dclose(dev);
	i = 0;
#endif
	for (i = 0; i < 128; ++i) {
		kprintf("%02x ", (int)(unsigned char)buf[i]);
		if ((i+1)%16 == 0) kprintf("\n");
	}

	void * addr = kmalloc(5);
	void * addr2 = kmalloc(6);
	kprintf("Kmalloc(5) == %x, Kmalloc(6)==%x\n", addr, addr2);
	kfree(addr2);
	/*kfree(addr);*/
	kprintf("Kmalloc(4) == %x, Kmalloc(6)==%x\n", kmalloc(4), kmalloc(6));
	int * num = kmalloc(4);

	*num = 5;

	/*strcpy(buf,"Moi, nyt on menossa hassu kirjoitustesti\n");
	dev = dopen(&fd_devices[1]);
	dwrite(buf, 1, 256, dev);
	dflush(dev);
	dclose(dev);*/

	/*char * memory = alloc_page();
	print("Allocated memory, and we got address: ");
	print_hex((unsigned int)memory);
	print("\n");
	free_page(memory);
	memory = alloc_pages(2);
	print("Allocated more memory, and we got address: ");
	print_hex((unsigned int)memory);
	print("\n");
	memory = alloc_page();
	print("Allocated even more memory, and we got address: ");
	print_hex((unsigned int)memory);
	print("\n");
	free_page(memory);*/

	kprintf("%s %s is up and running _o/\n", systeemi, versio);
	start_threading();
	//for (;;);
}
