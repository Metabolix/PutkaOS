#include <keyboard.h>
#include <screen.h>
#include <string.h>

#include <sh.h>

int sh_colour = 7;
char history[SH_HISTORY_SIZE][128];

void run_sh(void)
{
	int ch;
	char buffer[128];
	char *bufptr;
	const int buffer_size = 120;
	int loc;

	struct sh_komento *komento;

	int history_index = -1;
	int h;
	for (h = 0; h < SH_HISTORY_SIZE; h++) {
		history[h][0] = 0;
	}

	for(;;) {
		loc = 0;
		buffer[0] = 0;
		set_colour(sh_colour);
		print("PutkaOS $ ");
		while (1) {
			ch = kb_get();
			if (ch & 256) { /* Key up */
				continue;
			}

			/* Up arrow */
			if (ch == KEY_UP) {
				/* If the history is boring (empty) */
				if (history_index >= SH_HISTORY_SIZE - 1 || history[history_index+1][0] == 0) {
					continue;
				}
				history_index++;
				/* Clear */
				while (loc > 0) {
					print("\b \b");
					buffer[--loc] = 0;
				}
				strcpy(buffer, history[history_index]);
				kprintf("%s", buffer);
				loc = strlen(buffer);
				continue;
			}

			/* Down arrow */
			if (ch == KEY_DOWN) {
				if (history_index >= 0) {
					/* Clear */
					while (loc > 0) {
						print("\b \b");
						buffer[--loc] = 0;
					}
					if (history_index) {
						history_index--;
						strcpy(buffer, history[history_index]);
						kprintf("%s", buffer);
						loc = strlen(buffer);
					} else {
						history_index--;
					}
				} else { /* Clear */
					while (loc > 0) {
						print("\b \b");
						buffer[--loc] = 0;
					}
				}
				continue;
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

		ajettu: {}
		/* Lets add this line at the beginning of the history list */
		int i;
		/* First move every command from the list one step down and drop the lastest one */
		for (i = SH_HISTORY_SIZE - 2; i >= 0; i--) {
			strcpy(history[i+1], history[i]);
		}
		/* And then save new command and do some other usefull stuff */
		history_index = -1;
		strcpy(history[0], buffer);

	}
}
