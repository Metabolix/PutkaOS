#include <irq.h>
#include <screen.h>
#include <idt.h>
#include <timer.h>
#include <io.h>
#include <floppy.h>
#include <mem.h>

static char seconds = 0;
static unsigned long int minutes = 0;
static unsigned int ticks = 0;
struct timer_job tjobs[20];
int job_count = 0;
const int tjobs_max_count = 20;


void execute_jobs()
{
	int a, jobs_tried = 0;
	void (*function)();

	for(a = 0; a < tjobs_max_count && jobs_tried < job_count; a++) {
		if(tjobs[a].function && tjobs[a].time == ticks) {
			jobs_tried++;

			if(tjobs[a].times) {
				tjobs[a].time += tjobs[a].freq;
				function = tjobs[a].function;
				function();
			}
			if(tjobs[a].times) {
				if(!(tjobs[a].times--)) {
					tjobs[a].function = 0;
				}
			}
		}
	}
}

void timer_handler()
{
	ticks++;

	if((ticks % HZ) == 0) {
		seconds++;
		if(seconds > 59) {
			minutes++;
			minutes - 1 ? kprintf("TIMER: %u minutes\n", minutes):kprintf("TIMER: %u minute\n", minutes);
			seconds = 0;
		}
		/*print_hex(seconds);
		print(" seconds\n");*/
	}

	execute_jobs();
}

unsigned int kget_ticks()
{
	return ticks;
}

void kregister_job(struct timer_job * job)
{
	int a;

	for(a = 0; a < tjobs_max_count; a++) {
		if(!tjobs[a].function) {
			tjobs[a] = (*job);
			job_count++;
			return;
		}
	}
	print("DEBUG: Couldn't register job\n");
}

void kunregister_job(struct timer_job * job)
{
	int a;

	for(a = 0; a < tjobs_max_count; a++) {
		if(tjobs[a].function == (*job).function) {
			tjobs[a].function = 0;
			job_count--;
			return;
		}
	}
}



void timer_install()
{
	extern void irq0();

	outportb(0x43, 0x34); // binary, mode 2, LSB/MSB, ch 0
	outportb(0x40, TIME & 0xff); // LSB
	outportb(0x40, TIME >> 8); // MSB

	memset(tjobs, 0, sizeof(struct timer_job) * 20);

	install_irq_handler(0, (void *)timer_handler);
}

void kwait(int ms)
{
	int cur_ticks = ticks;
	int ms_multiplier = 10;

	while(((ticks - cur_ticks) * ms_multiplier) < ms);
}
