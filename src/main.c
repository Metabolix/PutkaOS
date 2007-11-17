#include <misc_asm.h>
#include <gdt.h>
#include <irq.h>
#include <keyboard.h>
#include <memory/memory.h>
#include <memory/malloc.h>
#include <idt.h>
#include <gdt.h>
#include <isr.h>
#include <io.h>
#include <panic.h>
#include <timer.h>
#include <multiboot.h>
#include <sh.h>
#include <multitasking/multitasking.h>
#include <devices/devmanager.h>
#include <string.h>
//#include <mouse.h>
#include <filesys/mount.h>
#include <devices/blockdev/ide.h>
#include <devices/blockdev/floppy.h>
#include <devices/ports/serial.h>
#include <devices/ports/pci.h>
#include <devices/display/text/pc_display.h>
#include <devices/display/text/lcdscreen.h>
#include <vt.h>
#include <screen.h>

void testattava_koodi();

const char systeemi[] = "PutkaOS";
const char versio[] = "v. 0.006";
tid_t sh_tid;

multiboot_info_t mbt_real;
multiboot_info_t *mbt;

extern int sprintf(char * restrict str, const char * restrict fmt, ...);

size_t testfwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
	kprintf("testfwrite(): size=%d, count=%d: \"%s\"\n", size, count, buf);
	return count;
}

void parse_multiboot(multiboot_info_t *mbt)
{
	// Muisti
	if (mbt->flags & MBI_FLAG_MEMORY) {
	} else {
		mbt->mem_lower = mbt->mem_upper = 0;
	}

	// Käynnistyslaite
	if (mbt->flags & MBI_FLAG_BOOTDEV) {
	} else {
		memset(mbt->boot_device, 0, sizeof(mbt->boot_device));
	}

	// Komentorivi
	if (mbt->flags & MBI_FLAG_CMDLINE) {
		static char cmdline[128];
		strncpy(cmdline, mbt->cmdline, 127);
		mbt->cmdline = cmdline;
	} else {
		mbt->cmdline = 0;
	}

	if (mbt->flags & MBI_FLAG_MODS) {
	} else {
	}

	if (mbt->flags & MBI_FLAG_AOUT_SYMS) {
		panic("Ei me olla a.out, jookos? Tee ELF.");
	} else {
	}

	if (mbt->flags & MBI_FLAG_ELF_SECS) {
	} else {
	}

	// BIOSin muistikartta, jos sillä on asiaa
	if (mbt->flags & MBI_FLAG_BIOS_MMAP) {
	} else {
	}

	if (mbt->flags & MBI_FLAG_BIOS_DRIVES) {
	} else {
	}

	if (mbt->flags & MBI_FLAG_CONFIG_TABLE) {
	} else {
	}

	if (mbt->flags & MBI_FLAG_BOOT_LOADER_NAME) {
	} else {
	}

	if (mbt->flags & MBI_FLAG_APM) {
	} else {
	}

	if (mbt->flags & MBI_FLAG_VBE) {
	} else {
	}
}

void kmain(multiboot_info_t* param_mbt, unsigned int magic)
{
	mbt = &mbt_real;
	mbt_real = *param_mbt;
	parse_multiboot(mbt);
	vt_init(); //vt:t jäävät vielä fallback-tilaan
	cls();

	gdt_install();
	idt_install();
	isr_install();
	irq_install();
	init_syscalls();

	floppy_cp_mem();
	timer_install();

	memory_init(mbt->mem_upper + mbt->mem_lower);
	malloc_init();

	threading_init();

	keyboard_install();
	//mouse_install();

	fs_init();
	devmanager_init();

	vt_dev_init();
	serial_init();

	ide_init();
	pci_init();

	threading_start();
}
void kmain2(void)
{
	irq_unmask();

	floppy_init();
	floppy_reset();
	mount_init(mbt->boot_device, mbt->cmdline);

	//avataan oikea näyttöajuri ja asetetaan se vt-jutun käyttöön
	display_init();
	vt_setdriver("/dev/display");

	testattava_koodi();

	kprintf("%s %s is up and running _o/\n", systeemi, versio);
	sh_tid = new_thread(0, run_sh, 0, 0, 0);

	// Idle thread. ;)
	for (;;) switch_thread();
}

void testattava_koodi()
{
	print("<testattava_koodi>\n");
#if 0
	FILE *f;
	f = fopen("/dev/vt0", "r+");
	if(f==NULL) panic("apuva!");
	print("opened\n");
	fwrite("moi vt", 6, 1, f);
	print("printed\n");
	ioctl(f, IOCTL_VT_BLOCKMODE, VT_NOBLOCK);
	print("ioctl'd\n");
	char jee[10];
	fread(jee, 2, 1, f);
	print("read\n");
	kprintf("%i, %i", jee[0], jee[1]);
	fclose(f);
	print("closed\n");
#endif
#if 0
	time_t a, b, c, d;
	struct tm tm = {
		.tm_year = 70,
		.tm_mon = 0,
		.tm_mday = 1,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	};
	a = mktime(&tm);
	tm.tm_hour = 25;
	b = mktime(&tm);
	tm.tm_year = 75;
	c = mktime(&tm);
	tm.tm_sec = 75;
	d = mktime(&tm);
	kprintf("mktime diff:\n %d\n %d\n %d\n %d\n", a-1328076303, b-1328137503, c-160527903, d-160527975);
	kprintf("mktime:\n %d\n %d\n %d\n %d\n", a, b, c, d);
#endif
#if 0
	FILE *files[2];

	vt_get(files);

	FILE *vt_head1 = files[0];
	FILE *vt_head2 = files[1];

	char foo[] = "mese ";
	char foo2[] = "kebab ";
	char foo3[] = "8D";

	fwrite(foo, 1, strlen(foo)-1, vt_head1);
	fwrite(foo2, 1, strlen(foo2), vt_head1);
	fwrite(foo2, 1, strlen(foo2), vt_head2);
	fwrite(foo, 1, strlen(foo), vt_head2);

	char bar[20];

	fread(bar, 1, strlen(foo)-1, vt_head2);
	kprintf("\"%s\"\n", bar);

	ioctl(vt_head2, IOCTL_VT_SET_FWRITE, (uintptr_t)testfwrite);

	fwrite(foo3, 1, strlen(foo3), vt_head1);

	fread(bar, 1, strlen(foo)-1+strlen(foo2), vt_head1);
	kprintf("\"%s\"\n", bar);

	fclose(vt_head1);
	fclose(vt_head2);
#endif
#if 0
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
	print("</testattava_koodi>\n");
}

