#include <irq.h>
#include <screen.h>
#include <idt.h>
#include <timer.h>
#include <io.h>
#include <floppy.h>
#include <mem.h>

#define nop_func()

volatile struct tm sys_time = {0};
volatile struct timeval uptime;

#define MAX_TIMERS 16
struct timer timers[MAX_TIMERS];
int timer_count = 0;

void execute_jobs()
{
	int i, j;
	for (i = j = 0; j < timer_count; ++i) {
		if (!timers[i].active) {
			continue;
		}
		++j;
		if (timers[i].next_run.sec < uptime.sec ||
		(timers[i].next_run.sec == uptime.sec
		&& timers[i].next_run.sec < uptime.sec)) {
			continue;
		}

		timers[i].next_run.usec += timers[i].freq.usec;
		timers[i].next_run.sec += timers[i].freq.sec;
		timers[i].next_run.sec += (timers[i].next_run.usec / 1000000);
		timers[i].next_run.usec %= 1000000;
		timers[i].func();
		++timers[i].times_run;
		if (timers[i].times > 0 && timers[i].times_run == timers[i].times) {
			timers[i].active = 0;
			--timer_count;
		}
	}
}

void ktimer_stop(timer_id_t timer_id)
{
	int i;
	for (i = 0; i < MAX_TIMERS && timers[i].id != timer_id; ++i);
	if (i < MAX_TIMERS) {
		timers[i].active = 0;
		--timer_count;
	}
}

timer_id_t ktimer_start(void (*func)(void), unsigned int msec, int times)
{
	static timer_id_t retval;
	int i;
	if (timer_count == MAX_TIMERS) {
		return 0;
	}
	++timer_count;
	++retval;
	for (i = 0; timers[i].active; ++i);
	timers[i].active = 1;
	timers[i].times = times;
	timers[i].times_run = 0;
	timers[i].freq.sec = msec / 1000;
	timers[i].freq.usec = 1000 * (msec % 1000);
	timers[i].next_run = uptime;
	timers[i].next_run.usec += timers[i].freq.usec;
	timers[i].next_run.sec += timers[i].freq.sec;
	timers[i].next_run.sec += (timers[i].next_run.usec / 1000000);
	timers[i].next_run.usec %= 1000000;
	timers[i].func = func;
	timers[i].id = retval;
	return timers[i].id;
}

int days_in_month(int month, int year) { return (month != 1) ? (30 ^ (((month + 1) & 8) >> 3) ^ ((month + 1) & 1)) : ((((year + 1900) % 4 == 0) && (((year + 1900) % 100 != 0) || ((year + 1900) % 400 == 0))) ? 29 : 28); }

void sys_next_year()
{
	++sys_time.tm_year;
}

void sys_next_month()
{
	if (++sys_time.tm_mon == 12) {
		sys_time.tm_mon = 0;
		sys_time.tm_yday = 0;
		sys_next_year();
	}
}

void sys_next_week()
{
	/* Jaa-a... */
}

void sys_next_day()
{
	++sys_time.tm_yday;
	++sys_time.tm_mday;
	++sys_time.tm_wday;
	if (sys_time.tm_wday == 7) {
		sys_time.tm_wday = 0;
	}
	if (sys_time.tm_mday > days_in_month(sys_time.tm_mon, sys_time.tm_year)) {
		sys_time.tm_mday = 0;
		if (sys_time.tm_wday) {
			sys_next_week();
		}
		sys_next_month();
	} else if (sys_time.tm_wday) {
		sys_next_week();
	}
}

void sys_next_hour()
{
	if (++sys_time.tm_hour > 23) {
		sys_time.tm_hour -= 24;
		sys_next_day();
	}
}

void sys_next_minute()
{
	if (++sys_time.tm_min > 59) {
		sys_time.tm_min -= 60;
		sys_next_hour();
	}
	kprintf("Kello on %02u.%02u, uptime %u,%06u sekuntia.%c",
		sys_time.tm_hour, sys_time.tm_min,
		uptime.sec, uptime.usec, 0x0a);
	/* Joo, en saa escapetusmerkkiä enkä jaksa pasteta mistään. :P */
}

void timer_handler()
{
	static unsigned long ticks;
	ticks += TIMER_TICKS_PER_CYCLE;
	if (ticks > TIMER_TICK_RATE) {
		ticks -= TIMER_TICK_RATE;
		sys_time.tm_usec = uptime.usec = TICKS_TO_MICROSEC(ticks);
		++uptime.sec;
		if (++sys_time.tm_sec > 59) {
			sys_time.tm_sec -= 60;
			sys_next_minute();
		}
	} else sys_time.tm_usec = uptime.usec = TICKS_TO_MICROSEC(ticks);
	execute_jobs();
}

void timer_install()
{
	extern void irq0();

	outportb(0x43, 0x34); // binary, mode 2, LSB/MSB, ch 0
	outportb(0x40, TIMER_TICKS_PER_CYCLE & 0xff); // LSB
	outportb(0x40, TIMER_TICKS_PER_CYCLE >> 8); // MSB

	memset(timers, 0, sizeof(struct timer) * MAX_TIMERS);

	install_irq_handler(0, (void *)timer_handler);
}

void kwait(unsigned int msec)
{
	struct timeval jatkoaika = uptime;
	jatkoaika.usec += 1000 * (msec % 1000);
	jatkoaika.sec += msec / 1000;
	jatkoaika.sec += (jatkoaika.usec / 1000000);
	jatkoaika.usec %= 1000000;
	while (jatkoaika.sec > uptime.sec) nop_func();
	while (jatkoaika.sec == uptime.sec && jatkoaika.usec > uptime.usec) nop_func();
}
