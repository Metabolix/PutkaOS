#ifndef _MULTITASKING_H
#define _MULTITASKING_H 1

#include <memory/memory.h>
#include <stdint.h>
#include <stddef.h>
#include <irq.h>
#include <isr.h>

typedef void (*entry_t)(void);
typedef uint_t tid_t;
typedef uint_t pid_t;

struct thread;
struct process;

enum thread_state_t {
	TP_STATE_FREE = 0,
	TP_STATE_ALLOCED,
	TP_STATE_RUNNING,
	TP_STATE_SLEEPING,
	TP_STATE_ENDED,

	TP_STATE_ERR,
};

#define MAX_THREADS ((tid_t)(1<<10))
#define MAX_PROCESSES ((pid_t)(1<<10))
#define MAX_THREADS_PER_PROCESS (MAX_STACKS)

#define NO_PROCESS ((pid_t)-1)
#define NO_THREAD ((tid_t)-1)

extern pid_t active_pid;
extern tid_t active_tid;

extern struct process * active_process;
extern struct thread * active_thread;

extern size_t process_count;
extern size_t thread_count;

extern void next_thread(void);

extern void threading_init(void);
extern void start_threading(void);

extern int has_threading(void); /* Onko aloitettu */
extern int is_threading(void);  /* Onko päällä, siis HUOMIOI IRQ:t yms! TODO: vähän muuttunut kaikki... ;) */

extern pid_t new_process(const void *code, size_t code_size, uint_t entry_offset, const void * stack, size_t stack_size, int user);
extern tid_t new_thread(pid_t pid, entry_t entry, const void * stack, size_t stack_size, int user);

extern void kill_process(pid_t pid);
extern void kill_thread(tid_t tid);

#endif
