#ifndef _THREAD_H
#define _THREAD_H 1

#include <regs.h>
#include <stdint.h>
#include <stddef.h>

typedef void (*t_entry)(void);
typedef unsigned int thread_id_t;
typedef unsigned int process_id_t;

/**
 * struct thread_t - for thread information
 * @esp: stored esp value, points to registers that will be popped on continue
 * @stack: stack pointer returned by malloc
 * @process: owning process
 *
 * Badly unready
**/
struct thread_t {
	struct regs_t *esp;
	unsigned short ss, reserved_01;
	uint_t stack;
	process_id_t process;
	int running;
};

/**
 * struct process_t - for process information
 * @num_threads: number of threads
 * @is_running: is process running
 *
 * Badly unready
**/
struct process_t {
	size_t num_threads;
	thread_id_t main_thread;
	int running;
};

#define K_THREAD_STACK_SIZE ((size_t)(1<<13))
#define K_MAX_THREADS ((thread_id_t)(1<<10))
#define NO_THREAD ((thread_id_t)-1)
#define K_MAX_PROCESSES ((process_id_t)(1<<10))
#define NO_PROCESS ((process_id_t)-1)

extern struct process_t processes[K_MAX_PROCESSES];
extern struct thread_t threads[K_MAX_THREADS];

extern process_id_t active_process;
extern thread_id_t active_thread;

extern struct process_t * active_process_ptr;
extern struct thread_t * active_thread_ptr;

extern thread_id_t kernel_idle_process;
extern thread_id_t kernel_idle_thread;

extern size_t num_processes;
extern size_t num_threads;

extern process_id_t new_process(t_entry entry, void * initial_stack, size_t initial_stack_size);
extern thread_id_t new_thread(t_entry entry, void * initial_stack, size_t initial_stack_size);
extern void kill_thread(thread_id_t thread);
extern void start_threading(void);
extern void next_thread(void);

#endif
