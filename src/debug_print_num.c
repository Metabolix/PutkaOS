#include <screen.h>
static int debug_print_num_luku = 0;

void debug_print_num(void)
{
	kprintf("DEBUG_PRINT: %u *****\n", debug_print_num_luku);
	++debug_print_num_luku;
}
