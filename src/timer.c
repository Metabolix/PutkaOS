#include <irq.h>
#include <screen.h>
#include <idt.h>
#include <timer.h>
#include <io.h>
#include <mem.h>

#include <int64.h>
#define TICKS_TO_MICROSECS(ticks) (((uint64_t)1000000) * ticks / ((uint64_t)TIMER_TICK_RATE))

#define nop_func()

volatile struct tm sys_time = {0};
volatile struct timeval uptime;

void get_sys_time(struct tm *sys_time_ptr) {if(sys_time_ptr)*sys_time_ptr = sys_time;}
void get_uptime(struct timeval *uptime_ptr) {if(uptime_ptr)*uptime_ptr = uptime;}

#define MAX_TIMERS 16
struct timer timers[MAX_TIMERS];
int timer_count = 0;

void execute_jobs(void)
{
	int i, j;
	for (i = j = 0; j < timer_count; ++i) {
		if (!timers[i].active) {
			continue;
		}
		++j;
		if (timers[i].next_run.sec > uptime.sec) {
			continue;
		}
		if (timers[i].next_run.sec == uptime.sec && timers[i].next_run.usec > uptime.usec) {
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
	timers[i].id = retval;
	timers[i].func = func;
	timers[i].active = 1;
	timers[i].times = times;
	timers[i].times_run = 0;
	timers[i].freq.sec = msec / 1000;
	timers[i].freq.usec = 1000 * (msec % 1000);
	timers[i].next_run.usec = uptime.usec + timers[i].freq.usec;
	timers[i].next_run.sec = uptime.sec + timers[i].freq.sec + (timers[i].next_run.usec / 1000000);
	timers[i].next_run.usec %= 1000000;
	return timers[i].id;
}

//int days_in_month(int month, int year) { return (month != 1) ? (30 ^ (((month + 1) & 8) >> 3) ^ ((month + 1) & 1)) : ((((year + 1900) % 4 == 0) && (((year + 1900) % 100 != 0) || ((year + 1900) % 400 == 0))) ? 29 : 28); }
int karkausvuosi(int year) { return (((year + 1900) % 4 == 0) && (((year + 1900) % 100 != 0) || ((year + 1900) % 400 == 0))); }
int days_in_month(int month, int year) { year += month / 12; month %= 12; if (month < 0) { month += 12; year -= 1; } return (month != 1) ? (30 ^ (((month + 1) & 8) >> 3) ^ ((month + 1) & 1)) : ((((year + 1900) % 4 == 0) && (((year + 1900) % 100 != 0) || ((year + 1900) % 400 == 0))) ? 29 : 28); }

void sys_next_year(void)
{
	++sys_time.tm_year;
}

void sys_next_month(void)
{
	if (++sys_time.tm_mon == 12) {
		sys_time.tm_mon = 0;
		sys_time.tm_yday = 0;
		sys_next_year();
	}
}

void sys_next_week(void)
{
	/* Jaa-a... */
}

void sys_next_day(void)
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

void sys_next_hour(void)
{
	if (++sys_time.tm_hour > 23) {
		sys_time.tm_hour -= 24;
		sys_next_day();
	}
}

void sys_next_minute(void)
{
	if (++sys_time.tm_min > 59) {
		sys_time.tm_min -= 60;
		sys_next_hour();
	}
	/*kprintf("On %u.%u. vuonna %u ja kello on %02u.%02u.%02u; uptime %u,%06u sekuntia.\n",
		sys_time.tm_mday, sys_time.tm_mon + 1, sys_time.tm_year + 1900,
		sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec,
		uptime.sec, uptime.usec);*/
}

void timer_handler(void)
{
	static unsigned long ticks;
	ticks += TIMER_TICKS_PER_CYCLE;
	if (ticks > TIMER_TICK_RATE) {
		ticks -= TIMER_TICK_RATE;
		sys_time.tm_usec = uptime.usec = TICKS_TO_MICROSECS(ticks);
		++uptime.sec;
		if (++sys_time.tm_sec > 59) {
			sys_time.tm_sec -= 60;
			sys_next_minute();
		}
	} else {
		sys_time.tm_usec = uptime.usec = TICKS_TO_MICROSECS(ticks);
	}
	execute_jobs();
}

unsigned char cmos[128];
void read_cmos(void);
__asm__(
"read_cmos:\n"
"    xorl %ecx, %ecx\n"
"    cli\n"
"read_cmos_loop:\n"
"    movb %cl, %al\n"
"    outb %al, $0x70\n"
"    nop\n"
"    nop\n"
"    nop\n"
"    inb $0x71, %al\n"
"    movb %al, cmos(,%ecx,1)\n"
"    inc %ecx\n"
"    cmp $0x80, %ecx\n"
"    jne read_cmos_loop\n"
"    sti\n"
"    ret\n"
);

unsigned char xD(unsigned char a) // 0x64 => 64 xD
{
	return ((10*(a>>4))+(a&0x0f));
}

void timer_install(void)
{
	extern void irq0();

	outportb(0x43, 0x34); // binary, mode 2, LSB/MSB, ch 0
	outportb(0x40, TIMER_TICKS_PER_CYCLE & 0xff); // LSB
	outportb(0x40, TIMER_TICKS_PER_CYCLE >> 8); // MSB

	memset(timers, 0, sizeof(struct timer) * MAX_TIMERS);

	read_cmos();
	sys_time.tm_usec = 0;
	sys_time.tm_sec = xD(cmos[0x00]);
	sys_time.tm_min = xD(cmos[0x02]);
	sys_time.tm_hour = (cmos[0x04] & 0x80) ? ((xD(cmos[0x04] - 0x80) + 12) % 24) : xD(cmos[0x04]);
	sys_time.tm_mday = xD(cmos[0x07]);
	sys_time.tm_mon = xD(cmos[0x08]) - 1;
	sys_time.tm_year = xD(cmos[0x09]) + 100;
	install_irq_handler(0, (void *)timer_handler);
}

void kwait(time_t sec, time_t usec)
{
	struct timeval jatkoaika = uptime;
	jatkoaika.usec += usec;
	jatkoaika.sec += sec;
	jatkoaika.sec += (jatkoaika.usec / 1000000);
	jatkoaika.usec %= 1000000;
	extern void taikatemppu();
	while (jatkoaika.sec > uptime.sec) taikatemppu();
	while ((jatkoaika.sec == uptime.sec) && (jatkoaika.usec > uptime.usec)) taikatemppu();
}

int kwait_until_0(time_t sec, time_t usec, waitfunc_t until_0, waitparam_t param)
{
	struct timeval j = uptime;
	j.usec += usec;
	j.sec += sec;
	j.sec += (j.usec / 1000000);
	j.usec %= 1000000;
	while (j.sec > uptime.sec) {
		if (until_0(param) == 0) return 0;
	}
	while ((j.sec == uptime.sec) && (j.usec > uptime.usec)) {
		if (until_0(param) == 0) return 0;
	}
	return -1;
}
