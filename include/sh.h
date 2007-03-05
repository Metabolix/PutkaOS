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

void sh_cat(char *buf);
void sh_ls(char *buf);
void sh_help(char *buf);
void sh_uptime(char *buf);
void sh_exit(char *buf);
void sh_outportb(char *buf);
void sh_inportb(char *buf);
void sh_ei_tunnistettu(char *buf);
void sh_list_colours(char *buf);
void sh_set_colour(char *buf);
void sh_reset(char *buf);
void sh_key_names(char *buf);
void sh_history(char *buf);

#endif
