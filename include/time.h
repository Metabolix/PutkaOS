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

struct timeval {
	time_t sec;
	time_t usec;
};
#define TIMEVAL_VALIDATE(tv) { \
	while ((tv).usec >= 1000000) { \
		++(tv).sec; \
		(tv).usec -= 1000000; \
	} \
	while ((tv).usec < 0) { \
		--(tv).sec; \
		(tv).usec += 1000000; \
	} \
}
#define TIMEVAL_ADD(dest, add) { \
	(dest).sec += (add).sec; \
	(dest).usec += (add).usec; \
	while ((add).usec >= 1000000) { \
		++(dest).sec; \
		(add).usec -= 1000000; \
	} \
}
#define TIMEVAL_SUBST(end, start) { \
	(end).sec -= (start).sec; \
	(end).usec -= (start).usec; \
	if ((end).usec < 0) { \
		--(end).sec; \
		(end).usec += 1000000; \
	} \
}
#define TIMEVAL_CMP(first, second) ( \
	((first).sec - (second).sec) ? ((first).sec - (second).sec) : ( \
	((first).usec - (second).usec) ? ((first).usec - (second).usec) : 0) \
)

extern time_t mktime(struct tm *timeptr);
extern time_t time(time_t *timer);
/*
clock_t clock(void);
double difftime(time_t time0, time_t time1);
time_t mktime(struct tm *timeptr);
time_t time(time_t *timer);
char *asctime(const struct tm *timeptr);
... kesken ...
*/

#endif
