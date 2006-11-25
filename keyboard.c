#include <io.h>
#include <irq.h>
#include <idt.h>
#include <screen.h>
#include <panic.h>

void keyboard_handle() {
	/*static int enter_presses = 0;*/
	unsigned char scancode = inportb(0x60);
	if(scancode == 0x1c) {
		print("You pressed enter\n");
		/*if(!enter_presses++) {
			print("You should press enter again to confirm what you are doing\n");
		} else {
			panic("Enter pressed too many times!\n");
		}*/
	}
}

void keyboard_install() {
	extern void irq1();
	idt_set_gate(0x21, (unsigned)irq1, 0x08, 0x8E);
	install_irq_handler(1, (void *) keyboard_handle);
	print("Keyboard installed\n");
}

