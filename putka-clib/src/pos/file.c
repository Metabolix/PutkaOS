#include <stdio.h>
#include <pos/file.h>
#include <pos/syscalls.h>

FILE *fopen2(const char * filename, uint_t flags)
{
	return syscall_fopen2(filename, flags);
}

int ioctl(FILE * stream, int request, intptr_t param)
{
	ioctl_params_t sysparam = {
		.f = stream,
		.request = request,
		.param = param,
	};
	return syscall_ioctl(&sysparam);
}
