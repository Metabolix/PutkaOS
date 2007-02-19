#include <keyboard.h>
#include <screen.h>
#include <timer.h>
#include <string.h>

extern struct tm sys_time;
extern struct timeval uptime;


void run_sh() {
	unsigned int ch;
	char buffer[100];
	const int buffer_size = 100;
	int loc;

	for(;;) {
		loc = 0;
		buffer[0] = 0;
		print("PutkaOS $ ");
		while(loc ? buffer[loc - 1] != '\n' : 1) {
			if(loc == buffer_size - 1) { /* Buffer is full */
				break;
			}
			
			ch = kb_get();
			if(ch & 256) { /* Key up */
				continue;
			}
			ch = ktoasc(ch);
			if(ch == '\b') {
				if(loc > 0) {
					putch(ch);
					putch(' ');
					putch(ch);
					loc--;
				}
			} else {
				buffer[loc++] = ch;
				putch(ch);
			}
		}
		buffer[loc - 1] = 0;
		if(!strcmp(buffer, "uptime")) {
			kprintf("On %u.%u. vuonna %u ja kello on %02u.%02u.%02u; uptime %u,%06u sekuntia.\n",
				sys_time.tm_mday, sys_time.tm_mon + 1, sys_time.tm_year + 1900,
				sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec,
				uptime.sec, uptime.usec);
		}
		if(!strcmp(buffer, "exit")) {
			return;
		}
	}
}
