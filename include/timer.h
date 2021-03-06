#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>
#include <time.h>

#define TIMER_FREQUENCY 100
/* CLOCKS_PER_SEC */
#define TIMER_TICK_RATE 1193180
#define TIMER_TICKS_PER_CYCLE  ((TIMER_TICK_RATE + TIMER_FREQUENCY / 2) / TIMER_FREQUENCY)

typedef unsigned int timer_id_t;

struct timer {
	timer_id_t id;
	int active;
	void (*func)();
	struct timeval freq;
	int times, times_run;
	struct timeval next_run;
};

extern int syscall_get_sys_time(struct tm *sys_time_ptr);
extern int syscall_get_uptime(struct timeval *uptime_ptr);

extern void timer_install();

typedef uintptr_t waitparam_t;
typedef int (*waitfunc_t)(waitparam_t);
extern void kwait(time_t sec, time_t usec);
extern int kwait_until_0(time_t sec, time_t usec, waitfunc_t until_0, waitparam_t param);

extern unsigned int kget_ticks(void);
extern timer_id_t ktimer_start(void (*func)(void), unsigned int msec, int times);
extern void ktimer_stop(timer_id_t timer);

#endif
