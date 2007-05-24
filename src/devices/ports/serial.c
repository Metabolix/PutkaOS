#include <devices/ports/serial.h>
#include <stdint.h>
#include <screen.h>

int serial_init(void)
{
	int i;
	uint16_t *ports = (void*) 0x400;
	for (i = 0; i < 4; ++i) {
		kprintf("serial port %d @ %#06x\n", i+1, ports[i]);
	}
	return 0;
}
