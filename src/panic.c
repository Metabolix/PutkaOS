#include <screen.h>

void panic(const char * msg)
{
	print("Kernel panic: ");
	print(msg);
	asm("cli");
	for(;;);
}
