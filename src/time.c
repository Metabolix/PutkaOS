#include <time.h>
#include <timer.h>

extern int days_in_month(int month, int year);
extern int karkausvuosi(int year);

static struct tm global_tm;

int karkausvuosia(int year)
{
	// TODO: karkausvuosia: (lkm) järkevä algoritmi!
	int i;
	int k = 0;
	if (year < 70) {
		return 0;
	}
	for (i = 70; i <= year; ++i) {
		if (karkausvuosi(i)) ++k;
	}
	return k;
}

struct tm *gmtime(const time_t *timer)
{
	if (!timer) {
		return 0;
	}
	time_t t = *timer;
	if (t < 0) {
		return 0;
	}
	return 0;

	// TODO: struct tm *gmtime(const time_t *timer)

	struct tm tm;
	tm.tm_sec = t % 60; t /= 60;
	tm.tm_min = t % 60; t /= 60;
	tm.tm_hour = t % 24; t /= 24;

	global_tm = tm;
	return &global_tm;
}

struct tm *localtime(const time_t *timer)
{
	if (!timer) {
		return 0;
	}
	time_t t = *timer;
	// TODO: localtime: t -= 60 * 60 * (TIME_ZONE_OFFSET)
	struct tm *tm = gmtime(&t);
	if (!tm) return 0;
	// TODO: localtime: tm->tm_isdst = ???; && tässä jatkosäätö tai -1h ja uusi gmtime-kutsu
	return tm;
}

time_t mktime_nofix(const struct tm *timeptr)
{
	struct tm tm;
	if (!timeptr) return (time_t)(-1);

	tm = *timeptr;

	if (tm.tm_mon < 0) {
		tm.tm_year += (tm.tm_mon - 11) / 12;
		tm.tm_mon = 12 + (tm.tm_mon % 12);
	} else {
		tm.tm_year += tm.tm_mon / 12;
		tm.tm_mon %= 12;
	}
	// TODO: mktime_nofix: kunnon algoritmi!
	while (tm.tm_mday > days_in_month(tm.tm_mon, tm.tm_year)) {
		tm.tm_mday -= days_in_month(tm.tm_mon, tm.tm_year);
		if (tm.tm_mon == 11) {
			tm.tm_mon = 0;
			++tm.tm_year;
		} else {
			++tm.tm_mon;
		}
	}
	while (tm.tm_mday <= 0) {
		if (tm.tm_mon == 0) {
			tm.tm_mon = 11;
			--tm.tm_year;
		} else {
			--tm.tm_mon;
		}
		tm.tm_mday += days_in_month(tm.tm_mon, tm.tm_year);
	}

	int64_t stamp_64 =
		+ tm.tm_sec
		+ tm.tm_min * 60
		+ tm.tm_hour * 60 * 60
		+ (tm.tm_mday - 1 + karkausvuosia(tm.tm_year)) * 24 * 60 * 60
		+ (tm.tm_year - 70) * 365 * 24 * 60 * 60;

	time_t stamp = stamp_64;
	if (stamp != stamp_64) {
		return (time_t)(-1);
	}
	return stamp;
}
#if 0
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
#endif

time_t mktime(struct tm *timeptr)
{
	struct tm tm, *tm_ptr;
	time_t stamp;

	if (!timeptr) {
		return (time_t)(-1);
	}
	tm = *timeptr;
	// tm.tm_hour -= TIME_ZONE_OFFSET
	stamp = mktime_nofix(&tm);
	if (stamp == (time_t)(-1)) {
		return stamp;
	}
	tm_ptr = localtime(&stamp);
	if (!tm_ptr) {
		return stamp;
		return (time_t)(-1);
	}
	*timeptr = *tm_ptr;
	return stamp;
}

time_t time(time_t *timer)
{
	struct tm tm;
	get_sys_time(&tm);
	if (!timer) {
		return mktime(&tm);
	}
	return (*timer = mktime(&tm));
}
