#ifndef _PROCESS_H
#define _PROCESS_H 1

#include <multitasking/multitasking.h>

/**
 * struct process - for process information
 * @threads: info about threads
 * @state: running or sleeping
 *
 * Badly unready
**/
struct process {
	struct {
		uint_t phys_pd;
	} mem;

	uint_t uid, gid;
	uint_t state;

	struct {
		tid_t tid0;
		size_t count;
		size_t freenum;
	} threads;

	int vt_num; /* < 0 if no vt */
};

extern struct process processes[MAX_PROCESSES];
extern size_t process_count;

extern int process_alloc_thread_num(pid_t pid);
extern void process_free_thread_num(pid_t pid, int thread_of_proc);
extern void free_process(pid_t pid);
extern void clean_processes(void);

#endif
