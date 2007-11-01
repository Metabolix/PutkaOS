#include <idt.h>
#include <screen.h>
#include <debug.h>

#include <syscall.h>
#include <memory/malloc.h>

uint_t asm_syscall();

void init_syscalls(void)
{
	idt_set_gate(0x80, (uintptr_t)asm_syscall, 0x08, 0x8E);
}

/**
* Syscalls
* (List in the end...)
**/

/**
* syscall_print: print(ebx);
**/
int syscall_print(uint_t eax, const char *ebx)
{
	print(ebx);
	return 0;
}

/**
* syscall_malloc: malloc(ebx);
**/
void * syscall_malloc(uint_t eax, uint32_t ebx)
{
	return malloc(ebx);
}

/**
* syscall_free: free(ebx);
**/
int syscall_free(uint_t eax, void *ebx)
{
	free(ebx);
	return 0;
}

/**
* syscall_fopen: fopen(ebx, ecx)
**/
FILE * syscall_fopen(uint_t eax, const char *name, const char *mode)
{
	return fopen(name, mode);
}

/**
* syscall_fclose: fclose(ebx);
**/
int syscall_fclose(uint_t eax, FILE *f)
{
	return fclose(f);
}

struct syscall_freadwrite {
	void *buf;
	size_t size;
	size_t count;
	FILE *f;
};

/**
* syscall_fread: fread(ebx[0], ebx[1], ebx[2], ebx[3]);
**/
size_t syscall_fread(uint_t eax, struct syscall_freadwrite *ebx)
{
	return fread(ebx->buf, ebx->size, ebx->count, ebx->f);
}

/**
* syscall_fwrite: fwrite(ebx[0], ebx[1], ebx[2], ebx[3]);
**/
size_t syscall_fwrite(uint_t eax, struct syscall_freadwrite *ebx)
{
	return fwrite(ebx->buf, ebx->size, ebx->count, ebx->f);
}

/**
* syscall_table: function pointers for asm to call.
**/
const syscall_t syscall_table[] = {
	(syscall_t)  /*    0 */ syscall_print
	,(syscall_t) /*    1 */ syscall_malloc
	,(syscall_t) /*    2 */ syscall_free

	,(syscall_t) /*    3 */ syscall_fopen
	,(syscall_t) /*    4 */ syscall_fclose
	,(syscall_t) /*    5 */ syscall_fread
	,(syscall_t) /*    6 */ syscall_fwrite
};
const syscall_t *syscall_table_ptr = syscall_table;
const int syscall_table_size = (sizeof(syscall_table) / sizeof(syscall_t));
