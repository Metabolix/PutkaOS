#ifndef _TIME_H
#define _TIME_H 1

#include <stddef.h>

typedef size_t time_t;
typedef size_t clock_t;

#define CLOCKS_PER_SEC ((clock_t) 1193180 )

struct tm {
	int tm_usec; /* Not in C99 standard! */
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

/*
clock_t clock();
double difftime(time_t time0, time_t time1);
time_t mktime(struct tm *timeptr);
time_t time(time_t *timer);
char *asctime(const struct tm *timeptr);
... kesken ...
*/

#endif
