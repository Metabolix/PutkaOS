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
#include <devices/devmanager.h>
#include <string.h>
//#include <mouse.h>
#include <filesys/mount.h>
#include <devices/blockdev/ide.h>
#include <devices/blockdev/floppy.h>
#include <devices/ports/serial.h>

void testattava_koodi();

const char systeemi[] = "PutkaOS";
const char versio[] = "v0.002";

multiboot_info_t mbt_real;
multiboot_info_t *mbt = &mbt_real;

extern int sprintf(char * restrict str, const char * restrict fmt, ...);

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

	fs_init();
	devmanager_init();
	floppy_init();
	vts_init();
	serial_init();
	threading_init();
	init_syscalls();
	ide_init();

	kprintf("Going to unmask irqs\n");
	outportb(0x21, 0); /* Don't mask any IRQ */
	outportb(0xa1, 0);
	asm_sti(); /* Allow interrupts */

	//floppy_reset();
	mount_init(mboot_device, mboot_cmdline);
	print("testattava_koodi();\n");
	testattava_koodi();

	kprintf("%s %s is up and running _o/\n", systeemi, versio);
	new_thread(run_sh, 0, 0);
	start_threading();
}

void testattava_koodi()
{
	char b[128];
	int i = 123;
#define P(x) sprintf(b, x, i); kprintf("%20s: '%s'\n", x, b);
	P("%d")
	P("%0d")
	P("%+d")
	P("% d")

	P("%8d")
	P("%08d")
	P("%+08d")
	P("% 08d")
	P("% 8d")
	P("%08d")
	P("%.8d")
	P("%+.8d")
	P("%-.8d")
	P("% 8.4d")
	P("%+8.4d")
	P("%-+8.4d")
	P("% 08.4d")
	P("%+08.4d")
#if 0
	FILE *file = fopen("/aja", "r");
	if(file) {
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		char * pointer = malloc(size);
		fseek(file, 0, SEEK_SET);
		fread(pointer, size, 1, file);
		fclose(file);
		new_process(pointer, 0, 0, 1, size);
	} else {
		print("Couldn't read program from filesystem!\n");
	}
#endif
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

