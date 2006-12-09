#include <thread.h>
#include <malloc.h>
#include <string.h>
#include <panic.h>

/* TODO: MALLOC & FREE */
#define free(a)
#define malloc(a) alloc_pages(a)

void process_ending(void)
{
}

process_id_t find_free_process(void)
{
	static process_id_t free_process, alk_free_process;
	alk_free_process = free_process;
	do {
		if (processes[free_process].num_threads == 0) {
			alk_free_process = free_process;
			free_process = (free_process + 1) & (K_MAX_PROCESSES - 1); // % K_MAX_THREADS
			return alk_free_process;
		}
		free_process = (free_process + 1) & (K_MAX_PROCESSES - 1); // % K_MAX_THREADS
	} while (free_process != alk_free_process);

	return NO_PROCESS;
}

process_id_t find_running_process(void)
{
	static process_id_t i;
	if (num_processes != 0) for (i = 0; i < K_MAX_PROCESSES; ++i) {
		if (processes[i].running) {
			return i;
		}
	}
	return NO_THREAD;
}

process_id_t new_process(t_entry entry, void * initial_stack, size_t initial_stack_size)
{
	process_id_t process;
	process = find_free_process();
	if (process == NO_PROCESS) {
		return NO_PROCESS;
	}
	processes[process].main_thread = new_thread(entry, initial_stack, initial_stack_size);
	if (processes[process].main_thread == NO_THREAD) {
		return NO_PROCESS;
	}
	processes[process].num_threads = 1;
	processes[process].running = 1;
	return process;
}
