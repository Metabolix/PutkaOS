#include <sys/syscalls.h>
#include <stddef.h>
#include <stdint.h>

extern intptr_t mksyscall(uint_t eax, ...);

int syscall_print(const char *text)
{
	return mksyscall(SYSCALL_PRINT, text);
}

void *syscall_malloc(size_t size)
{
	return (void*) mksyscall(SYSCALL_MALLOC, size);
}

void syscall_free(void *ptr)
{
	mksyscall(SYSCALL_FREE, ptr);
}

void *syscall_realloc(void *ptr, size_t new_size)
{
	return (void*) mksyscall(SYSCALL_REALLOC, ptr, new_size);
}

int syscall_get_system_time(struct tm *sys_time_ptr)
{
	return mksyscall(SYSCALL_GET_SYSTEM_TIME, sys_time_ptr);
}

int syscall_get_uptime(struct timeval *uptime_ptr)
{
	return mksyscall(SYSCALL_GET_UPTIME, uptime_ptr);
}

FILE * syscall_fopen2(const char *filename, uint_t flags)
{
	return (FILE*) mksyscall(SYSCALL_FOPEN2, filename, flags);
}

int syscall_fclose(FILE *f)
{
	return mksyscall(SYSCALL_FCLOSE, f);
}

size_t syscall_fread(fread_params_t *params)
{
	return mksyscall(SYSCALL_FREAD, params);
}

size_t syscall_fwrite(fwrite_params_t *params)
{
	return mksyscall(SYSCALL_FWRITE, params);
}

int syscall_fseek(fseek_params_t *params)
{
	return mksyscall(SYSCALL_FSEEK, params);
}

int syscall_fflush(FILE *f)
{
	return mksyscall(SYSCALL_FFLUSH, f);
}

int syscall_fgetpos(FILE *f, fpos_t *pos)
{
	return mksyscall(SYSCALL_FGETPOS, f, pos);
}

int syscall_fsetpos(FILE *f, const fpos_t *pos)
{
	return mksyscall(SYSCALL_FSETPOS, f, pos);
}

int syscall_ioctl(ioctl_params_t *param)
{
	return mksyscall(SYSCALL_IOCTL, param);
}
