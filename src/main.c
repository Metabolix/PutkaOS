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
#include "multiboot.h"

void say_hello() {
	print("Hello\n");
}

void kmain(multiboot_info_t* mbt,unsigned int magic)
{
	cls();
	if((mbt->flags & 1) == 0)
		panic("Mbt->flags bit 1 wasn't 1\n");
	gdt_install();
	idt_install();
	isrs_install();
	init_memory(mbt->mem_upper + mbt->mem_lower);
	install_irq();
	keyboard_install();
	timer_install();
	install_floppy();
	


	outportb(0x21,0x0); /* Don't mask any interrupt */
	outportb(0xa1,0x0);
   	asm __volatile__("sti");	/* Allow interrupts */
	

	/* nice job testing :) */
	/*{
		struct timer_job job;
		job.function = &say_hello;
		job.times = -1;
		job.time = kget_ticks() + 100;
		job.freq = 50;

		kregister_job(&job);
	}*/

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



	print("PutkaOS v0.001 is up and running _o/\n");

	while(1);
}