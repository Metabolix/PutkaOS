#include <stdint.h>
#include <stdio.h>

typedef struct fread_params {
	void *buf;
	size_t size;
	size_t count;
	FILE *f;
} fread_params_t;

/**
* syscall_fopen: fopen(ecx, edx)
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

/**
* syscall_fread: fread(ecx[0], ecx[1], ecx[2], ecx[3]);
**/
size_t syscall_fread(uint_t eax, fread_params_t *ecx)
{
	return fread(ecx->buf, ecx->size, ecx->count, ecx->f);
}

/**
* syscall_fwrite: fwrite(ecx[0], ecx[1], ecx[2], ecx[3]);
**/
size_t syscall_fwrite(uint_t eax, fread_params_t *ebx)
{
	return fwrite(ebx->buf, ebx->size, ebx->count, ebx->f);
}
