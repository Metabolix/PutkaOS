#ifndef _SCHEDULER_H
#define _SCHEDULER_H 1

extern void scheduler(void);
extern void switch_thread(void);
extern void syscall_switch_thread(void);
extern void select_next_thread(void);

#endif
