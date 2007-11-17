#include <kprintf.h>
#include <misc_asm.h>

void panic(const char * msg)
{
	kprintf("Kernel panic: ");
	kprintf(msg);
	asm_cli();
	for(;;) asm_hlt();
}
