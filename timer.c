#include <irq.h>
#include <screen.h>
#include <idt.h>
#include <timer.h>
#include <io.h>
#include <floppy.h>

static char seconds = 0;
static long int minutes = 0;
static int ticks = 0;

void timer_handler() {
	ticks++;

	if((ticks % HZ) == 0) {
		seconds++;
		if(seconds > 59) {
			minutes++;
			print_hex(minutes);
			print(" minutes\n");
			seconds = 0;
		}
	}
}


void timer_install() {
	extern void irq0();

	outportb(0x43, 0x34); // binary, mode 2, LSB/MSB, ch 0 
	outportb(0x40, TIME & 0xff); // LSB 
	outportb(0x40, TIME >> 8); // MSB 

	install_irq_handler(0, (void *)timer_handler);
}

void wait(int ms) {
	int cur_ticks = ticks;
	int ms_multiplier = 1000 / HZ;

	while(((ticks - cur_ticks) * ms_multiplier) < ms);
}
