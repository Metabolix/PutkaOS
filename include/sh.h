#ifndef _SH_H
#define _SH_H 1

struct sh_komento {
	char *komento;
	char *kuvaus;
	void (*suoritus)(char *buf);
};

#define SH_HISTORY_SIZE 20

extern struct sh_komento *komennot;
extern char history[SH_HISTORY_SIZE][128];
extern int sh_colour;
extern void run_sh(void);

#endif
