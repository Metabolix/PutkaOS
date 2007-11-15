#include <time.h>
#include <sys/time.h>
#include <sys/syscalls.h>

int get_system_time(struct tm *sys_time_ptr)
{
	if (!sys_time_ptr) {
		return -1;
	}
	return syscall_get_system_time(sys_time_ptr);
}

int get_uptime(struct timeval *uptime_ptr)
{
	if (!uptime_ptr) {
		return -1;
	}
	return syscall_get_uptime(uptime_ptr);
}
