#include <idt.h>
#include <screen.h>

#include <syscall.h>
#include <memory/malloc.h>

int asm_syscall();

void init_syscalls(void)
{
	idt_set_gate(0x80, (uintptr_t)asm_syscall, 0x08, 0x8E);
}

/**
* Syscalls
* (List in the end...)
**/

/**
* syscall_print: print((char*) ebx);
**/
intptr_t syscall_print(int eax, char *ebx, ...)
{
	print(ebx);
	return 0;
}

/**
* syscall_malloc: malloc(ebx);
**/
intptr_t syscall_malloc(int eax, intptr_t ebx, ...)
{
	return (intptr_t) malloc(ebx);
}

/**
* syscall_free: free((void*) ebx);
**/
intptr_t syscall_free(int eax, intptr_t ebx, ...)
{
	free((void*) ebx);
	return 0;
}

/**
* syscall_table: function pointers for asm to call.
**/
const syscall_t syscall_table[] = {
	(syscall_t)  /*    0 */ syscall_print
	,(syscall_t) /*    1 */ syscall_malloc
	,(syscall_t) /*    2 */ syscall_free
};
const syscall_t *syscall_table_ptr = syscall_table;
const int syscall_table_size = (sizeof(syscall_table) / sizeof(syscall_t));
