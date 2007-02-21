#include <keyboard.h>
#include <screen.h>
#include <timer.h>
#include <string.h>
#include <panic.h>
#include <io.h>

#define HISTORY_SIZE 20

extern struct tm sys_time;
extern struct timeval uptime;

void sh_help(char *buf);
void sh_uptime(char *buf);
void sh_exit(char *buf);
void sh_outportb(char *buf);
void sh_inportb(char *buf);
void sh_ei_tunnistettu(char *buf);
void sh_list_colours(char *buf);
void sh_set_colour(char *buf);
void sh_reset(char *buf);
void sh_history(char *buf);

int sh_colour = 7;
char history[HISTORY_SIZE][128];

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
	{"lscolours", "Listaa varit", sh_list_colours},
	{"colour", "Aseta vari", sh_set_colour},
	{"reset", "Tyhjenna ruutu ja aseta perusvari", sh_reset},
	{"outp", "outb port byte, laheta tavu porttiin", sh_outportb},
	{"inp", "inp port byte, hae tavu portista", sh_inportb},
	{"history", "Komentohistoria", sh_history},
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

void sh_reset(char *buf)
{
	cls();
	sh_colour = 7;
}

void sh_set_colour(char *buf)
{
	int colour;
	while (*buf && !(*buf <= '9' && *buf >= '0')) ++buf;
	if (!*buf) return;
	colour = sh_read_int(&buf);
	if (colour < 0 || colour > 255) return;
	sh_colour = colour;
}

void sh_list_colours(char *buf)
{
	int i, j;
	set_colour(8);
	print("    ");
	for (j = 0; j < 16; ++j) {
		kprintf("x%02x ", j);
	}
	putch('\n');
	for (i = 0; i < 256; i += 16) {
		kprintf("x%02x ", i);
		for (j = 0; j < 16; ++j) {
			set_colour(i+j);
			kprintf("x%02x ", i+j);
		}
		set_colour(8);
		putch('\n');
	}
}

void sh_exit(char *buf)
{
	char vari[256] = {0};
	vari[' '] = 0;
	vari['.'] = 0x44;
	vari['$'] = 0xdd;
	vari['+'] = 0x99;
	static const char * dont_panic_xpm[] = {
	"  ++                 +        ++                                    ...   ",
	"+++++++     ++++     ++     + ++ ++++++++                         .....   ",
	"++    ++   ++  ++   +++    ++ ++ ++++++++++                 .     ..      ",
	"++     +   ++  ++   +++    +   +     ++                 .   .    ..       ",
	"++     ++ ++   ++  ++ ++  ++        +++           .     .  ..    .        ",
	"++     ++ ++   ++ ++  ++  +         ++           ...    .   ..  ..        ",
	"++     +  +    ++ ++   + ++         ++   ..       ..    .   ..  ..        ",
	"++    ++  +    +  ++   +++          ++   ...      ...   .   ..   ..     ..",
	"++   ++   ++++++ ++    +++       ...$+   .. .     .. .  .    .    .. .... ",
	"++ +++     ++++  ++     +      .....$$.  .   .     .  ....   ..    .....  ",
	"+++++                        ...    +..  .   ..    .   ...   ..           ",
	"                             ..      ..  .   ...   ..   ..    .           ",
	"                             ..      ..  .  .....  ..    .    .           ",
	"                              ..      .  ...    .. ..    .                ",
	"                              ..    ..   .       .  .                     ",
	"                               ......    .        .                       ",
	"                               ....      .                                ",
	"                                ..       .                                ",
	"                                ..                                        ",
	"                                 .                                        ",
	"                                 ..                                       ",
	"                                  .                                       ",
	0};
	const char **rivi, *merkki;
	putch('\n');
	for (rivi = dont_panic_xpm; *rivi; ++rivi) {
		print("  ");
		for (merkki = *rivi; *merkki; ++merkki) {
			set_colour(vari[(unsigned char)*merkki]);
			putch(' ');
			while (merkki[0] == merkki[1]) {
				putch(' ');
				++merkki;
			}
		}
		print("\n");
		set_colour(8);
	}
	putch('\n');
	panic("exit kutsuttu!");
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

void sh_history(char *buf)
{
	int i;
	for (i=0; i<HISTORY_SIZE; i++) {
		if (history[i][0]=='\0')
			return;
		kprintf("%s\n", history[i]);
	}
}


void run_sh(void)
{
	int ch;
	char buffer[128];
	char *bufptr;
	const int buffer_size = 120;
	int loc;
	int history_index=-1;
	int h;
	for (h=0; h < HISTORY_SIZE; h++) 
		history[h][0]='\0';

	for(;;) {
		loc = 0;
		buffer[0] = 0;
		set_colour(sh_colour);
		print("PutkaOS $ ");
		while (buffer[loc] != '\n') {
			ch = kb_get();
			if (ch & 256) { /* Key up */
				continue; 
			}
			
			/* Up arrow */
			if (ch == 0xc8) {
				/* If the history is boring (empty) */
				if (history[history_index+1][0]=='\0') {
					continue;
				}
				history_index++;
				/* Clear */
				while(loc>0) {
					print("\b \b");
					buffer[--loc] = 0;
				}
				strcpy(buffer,history[history_index]);
				kprintf("%s", buffer);
				loc = strlen(buffer);

			}

			/* Down arrow */
			if (ch == 0xd0) {
				if (history_index>0) {
					/* Clear */
					while(loc>0) {
						print("\b \b");
						buffer[--loc] = 0;
					}
					history_index--;
					strcpy(buffer,history[history_index]);
					kprintf("%s", buffer);
					loc = strlen(buffer);
				}
				else if (history_index==0) { /* Clear */
					while(loc>0) {
						print("\b \b");
						buffer[--loc] = 0;
					}
				}
			}

			ch = ktoasc(ch);
			if (!ch) {
				continue;
			}
			if (ch == '\n') {
				break;
			}
			if (ch == '\b') {
				if (loc > 0) {
					print("\b \b");
					buffer[--loc] = 0;
				}
				continue;
			}
			if (loc < buffer_size - 1) {
				putch(ch);
				buffer[loc] = ch;
				buffer[++loc] = 0;
			}
		}
		putch('\n');

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
			if (strrmsame(komento->komento, buffer)[0]) {
				++komento;
				continue;
			}
			bufptr = strrmsame(buffer, komento->komento);
			if (!*bufptr || bufptr[0] == ' ') {
				komento->suoritus(bufptr);
				goto ajettu;
			}
			++komento;
		}
		komento->suoritus(buffer);

		/* Lets add this line at the beginning of the history list */
		int i;
		/* First move every command from the list one step down and drop the lastest one */
		for (i=HISTORY_SIZE-2; i >=0;  i--) 
			strcpy(history[i+1], history[i]);
		/* And then save new command and do some other usefull stuff */
		history_index=-1;
		strcpy(history[0], buffer);
		
		ajettu: {}
	}
}
