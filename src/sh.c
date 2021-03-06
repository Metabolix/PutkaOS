#include <keyboard.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <screen.h>

#define QEMU_CURSES_AND_SSH 0

#include <sh.h>

int sh_colour = 7;
char history[SH_HISTORY_SIZE][128];

void run_sh(void)
{
	int ch;
	char buffer[128];
	char *bufptr;
	const int buffer_size = 120;
	int loc, i;

	struct sh_komento *komento;

	int history_index = -1;
	int h;
	for (h = 0; h < SH_HISTORY_SIZE; h++) {
		history[h][0] = 0;
	}

	for(;;) {
		loc = 0;
		buffer[0] = 0;
		printf("%s", "\x001b[mPutkaOS $ ");
		while (1) {
			fread(&ch, 1, 1, stdin);

			//ch = kb_get();
			//printf("[%x]", ch);

			//if (ch & 256) { /* Key up */
			//	continue;
			//}

			/* Up arrow */
			if (ch == KEY_UP) {
				/* If the history is boring (empty) */
				if (history_index >= SH_HISTORY_SIZE - 1 || history[history_index+1][0] == 0) {
					continue;
				}
				history_index++;
				/* Clear */
				while(buffer[loc]){
					loc++;
					putchar(' ');
				}
				while (loc > 0) {
					printf("%s", "\b \b");
					buffer[--loc] = 0;
				}
				strcpy(buffer, history[history_index]);
				printf("%s", buffer);
				loc = strlen(buffer);
				continue;
			}

			/* Down arrow */
			if (ch == KEY_DOWN) {
				if (history_index >= 0) {
					/* Clear */
					while(buffer[loc]){
						loc++;
						putchar(' ');
					}
					while (loc > 0) {
						printf("%s", "\b \b");
						buffer[--loc] = 0;
					}
					if (history_index) {
						history_index--;
						strcpy(buffer, history[history_index]);
						printf("%s", buffer);
						loc = strlen(buffer);
					} else {
						history_index--;
					}
				} else { /* Clear */
					while(buffer[loc]){
						loc++;
						putchar(' ');
					}
					while (loc > 0) {
						printf("%s", "\b \b");
						buffer[--loc] = 0;
					}
				}
				continue;
			}
			if(ch == KEY_LEFT){
				if(loc > 0){
					putchar('\b');
					loc--;
				}
				continue;
			}
			if(ch == KEY_RIGHT){
				if(buffer[loc] != 0){
					putchar(buffer[loc]);
					loc++;
				}
				continue;
			}
			if(ch == KEY_HOME){
				while(loc>0){
					printf("%s", "\b");
					loc--;
				}
				continue;
			}
			if(ch == KEY_END){
				while(buffer[loc]){
					putchar(buffer[loc]);
					loc++;
				}
				continue;
			}
			if(ch == KEY_DEL){
				if (loc >= 0 && buffer[loc] != 0) {
					for(i=loc; i < buffer_size-1; i++){
						buffer[i] = buffer[i+1];
						if(buffer[i] == 0) break;
						else putchar(buffer[i]);
					}
					printf("%s", " \b");
					for(i-=loc; i > 0; i--) putchar('\b');
				}
				continue;
			}
			//int hex = ch;
			//ch = ktoasc(ch);
			if (ch == '\n') {
				break;
			}
			if (!ch || ch == '\t') {
				continue;
			}
			if (ch == '\b') {
				if (loc > 0) {
					putchar('\b');
					loc--;
					for(i=loc; i < buffer_size-1; i++){
						buffer[i] = buffer[i+1];
						if(buffer[i] == 0) break;
						else putchar(buffer[i]);
					}
					printf("%s", " \b");
					for(i-=loc; i > 0; i--) putchar('\b');
				}
				continue;
			}

			if(ch > 0xff) continue; //joku tuntematon hassunäppäin

			if (loc < buffer_size - 1) {
				putchar(ch);
				//jos locin kohdalla on nolla, kirjoitetaan siihen merkki ja
				//nolla seuraavaan ja liikutaan
				if(buffer[loc]==0){
					buffer[loc] = ch;
					buffer[++loc] = 0;
					continue;
				}
				//haetaan bufferin pää ja samalla tulostellaan bufferin loppu
				for(i=loc; i < buffer_size; i++){
					if(buffer[i] == 0) break;
					putchar(buffer[i]);
				}
				//siirretään juttuja locin jälkeen eteen päin
				buffer[i+1] = 0;
				for(; i > loc; i--){
					buffer[i] = buffer[i-1];
					putchar('\b');
				}
				buffer[loc] = ch;
				loc++;
			}
		}
		putchar('\n');

#if QEMU_CURSES_AND_SSH
		for (i = 0; buffer[i]; ++i) {
			if (buffer[i] == '\'') {
				buffer[i] = '/';
			}
		}
#endif
		// Trim left
		for (loc = 0; buffer[loc] == ' '; ++loc);
		memmove(buffer, buffer + loc, strlen(buffer + loc) + 1);
		// Trim right
		loc = strlen(buffer) - 1;
		while (buffer[loc] == ' ' && loc >= 0) --loc;
		buffer[loc+1] = 0;
		// Check len
		if (loc == -1) continue;

		char *buf2 = malloc(strlen(buffer)*sizeof(char));
		strcpy(buf2, buffer);

		komento = komennot;
		while (komento->komento) {
			if (strrmsame(komento->komento, buffer)[0]) {
				++komento;
				continue;
			}
			bufptr = strrmsame(buffer, komento->komento);
			if (!*bufptr || bufptr[0] == ' ') {
				for (; *bufptr == ' '; ++bufptr);
				komento->suoritus(bufptr);
				goto ajettu;
			}
			++komento;
		}
		komento->suoritus(buffer);

		ajettu:;
		/* Lets add this line at the beginning of the history list */
		int i;
		/* First move every command from the list one step down and drop the lastest one */
		for (i = SH_HISTORY_SIZE - 2; i >= 0; i--) {
			strcpy(history[i+1], history[i]);
		}
		/* And then save new command and do some other usefull stuff */
		history_index = -1;
		strcpy(history[0], buf2);
		free(buf2);
	}
}
