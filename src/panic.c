#include <screen.h>
#include <putkaos.h>

void panic(const char * msg)
{
	print("Kernel panic: ");
	print(msg);
	asm_cli();
	for(;;);
}
