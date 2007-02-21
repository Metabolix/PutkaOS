#include <putkaos.h>
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
#include <sh.h>
#include <lcdscreen.h>
#include <thread.h>

const char systeemi[] = "PutkaOS";
const char versio[] = "v0.001";
BD_DESC *dev;
char buf[512];
int i;
/*
void testattava_koodi(void)
{
	struct timeval uptime;
	struct timeval uptime2;
	for (;;) {
		get_uptime(&uptime);
		get_uptime(&uptime2);
		kwait(1);
		if (uptime
		move(0,0);
		putch('m');
	}
	lcd_init(0x378);
	lcd_move(0,0);
	lcd_putch('m');
	lcd_putch('o');
	lcd_putch('i');
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
	extern void read_super_block(BD_DESC * dev);
	dev = dopen(&fd_devices[0]);
	read_super_block(dev);
	char * buffer = readfile("/mese/kebab");
	//kprintf("File hassu includes: %s\n", buffer);
	dclose(dev);
#endif

}
*/
void kmain(multiboot_info_t* mbt, unsigned int magic)
{
	cls();
	if ((mbt->flags & 1) == 0) {
		panic("Mbt->flags bit 1 wasn't 1\n");
	}
	screen_init();
	gdt_install();
	idt_install();
	isrs_install();
	memory_init(mbt->mem_upper + mbt->mem_lower);
	irq_install();
	timer_install();
	keyboard_install();
	install_floppy();
	malloc_init();
	vts_init();
	threading_init();

	kprintf("Going to unmask irqs\n");
	outportb(0x21,0x0); /* Don't mask any IRQ */
	outportb(0xa1,0x0);
	asm __volatile__("sti"); /* Allow interrupts */

	//reset_floppy();

	//testattava_koodi();

	kprintf("%s %s is up and running _o/\n", systeemi, versio);
	new_thread(run_sh, 0, 0);
	start_threading();
	//for (;;);
}
void __stack_chk_fail(void) {}
