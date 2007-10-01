#include <screen.h>
#include <misc_asm.h>

void panic(const char * msg)
{
	print("Kernel panic: ");
	print(msg);
	asm_cli();
	for(;;) asm_hlt();
}
