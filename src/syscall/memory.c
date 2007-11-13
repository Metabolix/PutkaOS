#include <stdint.h>
#include <stddef.h>
#include <memory/malloc.h>

/**
* syscall_malloc: malloc(ecx);
**/
void * syscall_malloc(uint_t eax, size_t ecx)
{
	return malloc(ecx);
}

/**
* syscall_free: free(ecx);
**/
int syscall_free(uint_t eax, void *ecx)
{
	free(ecx);
	return 0;
}
