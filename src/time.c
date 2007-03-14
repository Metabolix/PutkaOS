#include <time.h>
extern int days_in_month(int month, int year);
extern int karkausvuosi(int year);

static int put_in_range(int *value, int *borrow, int max)
{
	if (*value >= max) {
		*borrow += *value / max;
		*value %= max;
	} else if (*value < 0) {
		*borrow += (*value / max) - 1;
		*value = (*value % max) + max;
	} else {
		return 0;
	}
	return -1;
}

time_t mktime(struct tm *timeptr)
{
	struct tm tm;
	time_t stamp;
	int i;
	if (!timeptr) return (time_t)(-1);
	tm = *timeptr;
	if (tm.tm_sec != 60) {
		put_in_range(&tm.tm_sec, &tm.tm_min, 60);
	}
	put_in_range(&tm.tm_min, &tm.tm_hour, 60);
	put_in_range(&tm.tm_hour, &tm.tm_mday, 24);

	// TODO: Ei tällaista silmukkaa, kunnollinen algoritmi!
	while (tm.tm_mday > days_in_month(tm.tm_mon, tm.tm_year)) {
		tm.tm_mday -= days_in_month(tm.tm_mon, tm.tm_year);
		++tm.tm_mon;
	}
	while (tm.tm_mday <= 0) {
		--tm.tm_mon;
		tm.tm_mday += days_in_month(tm.tm_mon, tm.tm_year);
	}
	put_in_range(&tm.tm_mon, &tm.tm_year, 12);
	*timeptr = tm;
//kprintf("%d-%d-%d %d:%d:%d\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	// TODO: jos aikaleima ei mahdu time_t:hen, return (time_t)(-1)
	stamp = tm.tm_sec;
	stamp += 60 * tm.tm_min;
	stamp += 60 * 60 * tm.tm_hour;
	stamp += 60 * 60 * 24 * (tm.tm_mday - 1);
	for (i = 0; i < tm.tm_mon; ++i) {
		stamp += 60 * 60 * 24 * days_in_month(i, tm.tm_year);
	}
	for (i = 70; i < tm.tm_year; ++i) {
		stamp += 60 * 60 * 24 * (karkausvuosi(i) ? 366 : 365);
	}
	// TODO: tm_wday ja tm_yday
	timeptr->tm_wday = 1;
	timeptr->tm_yday = 1;
	return stamp;
}
