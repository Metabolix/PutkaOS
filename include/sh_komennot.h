#ifndef _SH_KOMENNOT_H
#define _SH_KOMENNOT_H 1

#include <stdint.h>

void sh_mount(char *buf);
void sh_mount_ro(char *buf);
void sh_remount(char *buf);
void sh_remount_ro(char *buf);
void sh_mount_real(char *dev_point, uint_t mode, int remount);
void sh_umount(char *buf);

void sh_cat(char *buf);
void sh_ls(char *buf);

void sh_uptime(char *buf);

void sh_outportb(char *buf);
void sh_inportb(char *buf);

void sh_ei_tunnistettu(char *buf);
void sh_list_colours(char *buf);
void sh_set_colour(char *buf);
void sh_reset(char *buf);
void sh_help(char *buf);
void sh_exit(char *buf);
void sh_history(char *buf);

void sh_key_names(char *buf);

void sh_reboot(char *buf);

#endif
