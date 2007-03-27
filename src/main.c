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
#include <devmanager.h>
#include <string.h>
#include <mouse.h>
#include <filesys/mount.h>

void testattava_koodi();

const char systeemi[] = "PutkaOS";
const char versio[] = "v0.001";

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
	mouse_install();

	devmanager_init();
	install_floppy();
	vts_init();
	threading_init();

	kprintf("Going to unmask irqs\n");
	outportb(0x21,0x0); /* Don't mask any IRQ */
	outportb(0xa1,0x0);
	asm __volatile__("sti"); /* Allow interrupts */

	reset_floppy();
	mount_init(mboot_device, mboot_cmdline);
	print("testattava_koodi();\n");
	testattava_koodi();

	kprintf("%s %s is up and running _o/\n", systeemi, versio);
	new_thread(run_sh, 0, 0);
	start_threading();
	//for (;;);
}

void testattava_koodi()
{
#if 0
	int i, j = 0, k = 0;
	struct timeval u1, u2;
	get_uptime(&u1);
	for (i = 0; i < 1000000; ++i) {
		while (inportb(0x3DA) & 8) ++j;
		while (!(inportb(0x3DA) & 8)) ++k;
	}
	get_uptime(&u2);
	TIMEVAL_SUBST(u2, u1);
	kprintf("%d,%d ; j = %d, k = %d\n", u2.sec, u2.usec, j, k);
	for(;;);
#elif 0
	int x = 0, y = 0;
	struct mouse_state state;
	for (;;) {
		mouse_get_state(&state);
		if (!state.dx && !state.dy) continue;
		x += state.dx;
		y += state.dy;
		kprintf("(%d, %d)\n", x, y);
	}
#elif 0
	FILE *f;
	char buf[512];
	memset(buf, 0, sizeof(buf));
	int i,j;
	f = fopen("/dev/fd1", "r");
	if (!f) panic("fd0 ei aukea!");
	kprintf("Luettiin %d laitteelta fd0\n", fread(buf, 1, 512, f));
	for (i = 0; i < 256; i += j) {
		for (j = 0; j < 24; ++j) {
			kprintf("%02x ", (int)(unsigned char)buf[i+j]);
		}
		kprintf("\n");
	}
	for (i = 0; i < 256; i += j) {
		for (j = 0; j < 64; ++j) {
			putch((int)(unsigned char)buf[i+j]);
		}
		kprintf("\n");
	}
	fclose(f);
#endif
}

