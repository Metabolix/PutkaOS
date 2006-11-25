#include <irq.h>
#include <screen.h>
#include <idt.h>
#include <timer.h>
#include <io.h>

void timer_handler() {
	static char seconds = 0;
	static long int minutes = 0;
	static int ticks = 0;

	ticks++;
	if((ticks % HZ) == 0) {
		seconds++;
		if(seconds > 59) {
			minutes++;
			print_hex(minutes);
			print(" minutes\n");
			seconds = 0;
		}
		print_hex(seconds);
		print(" seconds\na\nb\n");
	}
}


void timer_install() {
	extern void irq0();

	outportb(0x43, 0x34); // binary, mode 2, LSB/MSB, ch 0 
	outportb(0x40, TIME & 0xff); // LSB 
	outportb(0x40, TIME >> 8); // MSB 

	idt_set_gate(0x20, (unsigned)irq0, 0x08, 0x8E);
	install_irq_handler(0, (void *)timer_handler);
}

