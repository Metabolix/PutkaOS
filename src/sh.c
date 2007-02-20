#include <keyboard.h>
#include <screen.h>
#include <timer.h>
#include <string.h>
#include <panic.h>
#include <io.h>

extern struct tm sys_time;
extern struct timeval uptime;


void sh_help(char*);
void sh_uptime(char*);
void sh_exit(char*);
void sh_outportb(char*);
void sh_inportb(char*);
void sh_ei_tunnistettu(char*);

struct komento {
	char *komento;
	char *kuvaus;
	void (*suoritus)(char *buf);
};

struct komento *komento, komennot[] = {
	{"?", "Apua", sh_help},
	{"help", "Apua", sh_help},
	{"exit", "Paniikki", sh_exit},
	{"panic", "Paniikki", sh_exit},
	{"uptime", "Uptime", sh_uptime},
	{"outp", "outb port byte, laheta tavu porttiin", sh_outportb},
	{"inp", "inp port byte, hae tavu portista", sh_inportb},
	{0, 0, sh_ei_tunnistettu} /* Terminaattori */
};

int sh_read_int(char **bufptr)
{
	char *buf = *bufptr;
	int retval = 0;
	if (buf[0] == '0') {
		if (buf[1] == 'x') {
			++buf;
			while (++buf) if (*buf <= '9' && *buf >= '0') {
				retval = 16 * retval + *buf - '0';
			} else if (*buf <= 'f' && *buf >= 'a') {
				retval = 16 * retval + 10 + *buf - 'a';
			} else if (*buf <= 'F' && *buf >= 'A') {
				retval = 16 * retval + 10 + *buf - 'A';
			} else {
				break;
			}
		} else {
			while (++buf) if (*buf <= '7' && *buf >= '0') {
				retval = 8 * retval + *buf - '0';
			} else {
				break;
			}
		}
	} else {
		while (*buf && (*buf <= '9' && *buf >= '0')) {
			retval = 10 * retval + *buf - '0';
			++buf;
		}
	}
	*bufptr = buf;
	return retval;
}

void sh_help(char *buf)
{
	komento = komennot;
	while (komento->komento) {
		kprintf("%10s -- %s\n", komento->komento, komento->kuvaus);
		++komento;
	}
}

void sh_exit(char *buf)
{
	panic("exit kutsuttu!\n");
}

void sh_uptime(char *buf)
{
	kprintf("On %u.%u. vuonna %u ja kello on %02u.%02u.%02u; uptime %u,%06u sekuntia.\n",
		sys_time.tm_mday, sys_time.tm_mon + 1, sys_time.tm_year + 1900,
		sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec,
		uptime.sec, uptime.usec);
}

void sh_outportb(char *buf)
{
	unsigned int port = 0, byte = 0;
	while (*buf && !(*buf <= '9' && *buf >= '0')) ++buf;
	port = sh_read_int(&buf);
	while (*buf && !(*buf <= '9' && *buf >= '0')) ++buf;
	byte = sh_read_int(&buf);

	kprintf("Port %d (%#x), sending %d (%#04x)\n", port, port, byte, byte);
	outportb(port, byte);
}

void sh_inportb(char *buf)
{
	unsigned int port = 0, byte;
	while (*buf && !(*buf <= '9' && *buf >= '0')) ++buf;
	port = sh_read_int(&buf);
	byte = inportb(port);
	kprintf("Port %d (%#x): got %d (%#04x)\n", port, port, byte, byte);
}

void sh_ei_tunnistettu(char *buf)
{
	kprintf("Tunnistamaton komento (%s) Mutta 'help' auttaa!\n", buf);
}

void run_sh(void)
{
	unsigned int ch;
	char buffer[128];
	char *bufptr;
	const int buffer_size = 120;
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

		// Trim left
		for (loc = 0; buffer[loc] == ' '; ++loc);
		memmove(buffer, buffer + loc, strlen(buffer + loc) + 1);
		// Trim right
		loc = strlen(buffer) - 1;
		while (buffer[loc] == ' ' && loc >= 0) --loc;
		buffer[loc+1] = 0;
		// Check len
		if (loc == -1) continue;

		komento = komennot;
		while (komento->komento) {
			bufptr = strrmsame(buffer, komento->komento);
			if (!*bufptr || bufptr[0] == ' ') {
				komento->suoritus(bufptr);
				goto ajettu;
			}
			++komento;
		}
		komento->suoritus(buffer);
		ajettu: {}
	}
}
