#ifndef _SYSCALLS_H
#define _SYSCALLS_H 1

#include <stdint.h>

#define SYSCALL_TYPEDEFS 1

#define SYSCALL_MACRO(num, func, name, proto, ...) \
extern proto;
#include <sys/syscalls.list.h>
#undef SYSCALL_MACRO
/*
extern void syscall_print(const char *text);
extern void *syscall_malloc(size_t size);
extern void syscall_free(void *ptr);
extern void *syscall_realloc(void *ptr, size_t new_size);
extern int syscall_get_system_time(struct tm *sys_time_ptr);
extern int syscall_get_uptime(struct timeval *uptime_ptr);
extern FILE * syscall_fopen(const char *filename, const char *mode);
extern int syscall_fclose(FILE *f);
extern int syscall_fread(fread_params_t *params);
extern int syscall_fwrite(fwrite_params_t *params);
*/
#endif
