#include <doublefault.h>
#include <panic.h>

void double_fault_handler(void)
{
	panic("Double fault! (TODO)");
}
