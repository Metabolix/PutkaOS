#ifndef _TIMER_H
#define _TIMER_H

#include <time.h>

#define TIMER_FREQUENCY 1000UL
/* CLOCKS_PER_SEC */
#define TIMER_TICK_RATE 1193180UL
#define TIMER_TICKS_PER_CYCLE  ((TIMER_TICK_RATE + TIMER_FREQUENCY / 2UL) / TIMER_FREQUENCY)

typedef unsigned int timer_id_t;

struct timeval {
	time_t sec;
	time_t usec;
};

struct timer {
	timer_id_t id;
	int active;
	void (*func)();
	struct timeval freq;
	int times, times_run;
	struct timeval next_run;
};

extern void get_sys_time(struct tm *sys_time_ptr);
extern void get_uptime(struct timeval *uptime_ptr);

extern void timer_install();
extern void kwait(time_t sec, time_t usec);
extern unsigned int kget_ticks(void);
extern timer_id_t ktimer_start(void (*func)(void), unsigned int msec, int times);
extern void ktimer_stop(timer_id_t timer);

#endif
