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
#include "multiboot.h"

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

	outportb(0x21,0xfd - 1); /* Only keyboard interrupts */
	outportb(0xa1,0xff);
   	asm __volatile__("sti");	/* Allow interrupts */
	outportb(0x20, 0x20);

	
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
	while(1) asm("hlt");
}
