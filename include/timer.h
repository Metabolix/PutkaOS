#ifndef _TIMER_H
#define _TIMER_H

#define HZ 100 
#define CLOCK_TICK_RATE 1193180 
#define TIME  ((CLOCK_TICK_RATE + HZ/2) / HZ)

struct timer_job {
	unsigned int time; /* next tick count to execute job */
	int times; /* how many times to execute this job, -1 if no count */
	int freq;
	void * function; /* pointer to function which we will execute */
};


void timer_install();
void kwait(int ms);
unsigned int kget_ticks();
void kregister_job(struct timer_job * job);
void kunregister_job(struct timer_job * job);

#endif
