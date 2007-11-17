#ifndef _SYSTEM_TIME
#define _SYSTEM_TIME 1

#include <time.h>

extern int get_system_time(struct tm *sys_time_ptr);
extern int get_uptime(struct timeval *uptime_ptr);

#endif
