#include <screen.h>
//#include <vt.h>

int syscall_print(const char * string)
{
	return vt_print(vt_out_get(), string);
}

