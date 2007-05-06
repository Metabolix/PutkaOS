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
#include <multiboot.h>
#include <regs.h>
#include <sh.h>
#include <lcdscreen.h>
#include <thread.h>
#include <devmanager.h>
#include <string.h>
//#include <mouse.h>
#include <filesys/mount.h>
#include <storage/ide.h>
#include <storage/floppy.h>

void testattava_koodi();

const char systeemi[] = "PutkaOS";
const char versio[] = "v0.002";

multiboot_info_t mbt_real;
multiboot_info_t *mbt = &mbt_real;

void kmain(multiboot_info_t* param_mbt, unsigned int magic)
{
	mbt_real = *param_mbt;
	char mboot_cmdline[128] = "";
	unsigned long mboot_device = 0;
	cls();
	if ((mbt->flags & (1 << 0)) == 0) {
		panic("Mbt->flags bit 1 wasn't 1\n");
	}
	if ((mbt->flags & (1 << 1))) {
		mboot_device = mbt->boot_device;
	}
	if ((mbt->flags & (1 << 2))) {
		strncpy(mboot_cmdline, (char*) mbt->cmdline, 128);
	}
	screen_init();
	gdt_install();
	idt_install();
	isrs_install();
	memory_init(mbt->mem_upper + mbt->mem_lower);
	irq_install();
	timer_install();
	malloc_init();
	keyboard_install();
	//mouse_install();

	devmanager_init();
	install_floppy();
	vts_init();
	threading_init();

	ide_init();

	kprintf("Going to unmask irqs\n");
	outportb(0x21,0x0); /* Don't mask any IRQ */
	outportb(0xa1,0x0);
	asm_sti(); /* Allow interrupts */

	reset_floppy();
	mount_init(mboot_device, mboot_cmdline);
	print("testattava_koodi();\n");
	testattava_koodi();

	kprintf("%s %s is up and running _o/\n", systeemi, versio);
	new_thread(run_sh, 0, 0);
	start_threading();
}

void testattava_koodi()
{
#if 0
	int x = 0, y = 0;
	struct mouse_state state;
	for (;;) {
		mouse_get_state(&state);
		if (!state.dx && !state.dy) continue;
		x += state.dx;
		y += state.dy;
		kprintf("(%d, %d)\n", x, y);
	}
#endif
}

