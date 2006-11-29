#ifndef _TIMER_H
#define _TIMER_H

#define HZ 100 
#define CLOCK_TICK_RATE 1193180 
#define TIME  ((CLOCK_TICK_RATE + HZ/2) / HZ)

void timer_install();
void wait(int ms);

#endif
