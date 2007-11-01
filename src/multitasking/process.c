#include <multitasking/multitasking.h>
#include <multitasking/process.h>
#include <multitasking/thread.h>
#include <memory/memory.h>
#include <memory/kmalloc.h>
#include <string.h>
#include <panic.h>
#include <debug.h>

struct process processes[MAX_PROCESSES];
size_t process_count = 0;

pid_t active_pid = NO_PROCESS;
struct process * active_process = 0;

uint_t processes_ended = 0;
pid_t processes_ended_arr[MAX_PROCESSES];

static pid_t alloc_process(void);

void kill_process(pid_t pid)
{
	if (processes[pid].state == TP_STATE_FREE || processes[pid].state == TP_STATE_ENDED) {
		return;
	}
	if (pid == active_pid) {
	}
	tid_t tid;
	size_t count = processes[pid].threads.count;
	for (tid = 0; tid < MAX_THREADS && count; tid++) {
		if (threads[tid].pid == pid) {
			kill_thread(tid);
			count--;
		}
	}
	if (count) {
		// TODO: kill_process: Mitäs nyt tehdään, kun ei tapettu kaikkia säikeitä?
	}
	processes[pid].state = TP_STATE_ENDED;
	processes_ended_arr[processes_ended] = pid;
	++processes_ended;
	--process_count;
}

void free_process(pid_t pid)
{
	if (processes[pid].state == TP_STATE_ALLOCED) {
		if (processes[pid].threads.tid0) {
			kill_thread(processes[pid].threads.tid0);
		}
		goto clearing;
	}
	if (processes[pid].state != TP_STATE_ENDED) {
		panic("free_process: not ended!");
	}
	if (pid == active_pid) {
		panic("free_process: process active!");
	}
	clean_threads();
	if (processes[pid].threads.count) {
		// TODO: free_process: Mitäs nyt tehdään, kun ei vapautettu kaikkia säikeitä?
	}
clearing:
	if (processes[pid].mem.phys_pd) {
		free_pagedir(processes[pid].mem.phys_pd);
	}
	memset(&processes[pid], 0, sizeof(processes[pid]));
	processes[pid].state = TP_STATE_FREE;
}

void clean_processes(void)
{
	clean_threads();
	while (processes_ended) {
		--processes_ended;
		free_process(processes_ended_arr[processes_ended]);
	}
}

void process_ending(void)
{
	//panic("Jumalauta, ei kai prosessi voi loppua?!\n");
	kill_process(active_pid);
}

#if 0
void create_user_page_tables(pid_t pid)
{
	int a;
	int address = 0;
	processes[pid].pd = (page_entry_t **) kmalloc(MEMORY_PAGE_SIZE); /* TODO: use function which gives us one page */
	processes[pid].pd[0] = (page_entry_t *) kmalloc(MEMORY_PAGE_SIZE);
	processes[pid].pd[64] = (page_entry_t *) kmalloc(MEMORY_PAGE_SIZE);
	processes[pid].pd[65] = (page_entry_t *) kmalloc(MEMORY_PAGE_SIZE);

	memset(processes[pid].pd[64], 0, MEMORY_PAGE_SIZE);
	memset(processes[pid].pd[65], 0, MEMORY_PAGE_SIZE);

	for (a = 0; a < 511; a++, address += MEMORY_PAGE_SIZE) {
		processes[pid].pd[0][a] = KERNEL_PE(address);
	}
}
#endif

int process_alloc_thread_num(pid_t pid)
{
	++processes[pid].threads.count;
	return processes[pid].threads.freenum++;
}

void process_free_thread_num(pid_t pid, int thread_of_proc)
{
	--processes[pid].threads.count;
	resize_stack(processes[pid].mem.phys_pd, thread_of_proc, 0, 0);
}

pid_t alloc_process(void)
{
	static pid_t viimeksi_loytynyt;
	clean_processes();
	pid_t i = viimeksi_loytynyt;
	do {
		if (processes[i].state == TP_STATE_FREE) {
			memset(processes + i, 0, sizeof(processes[i]));
			processes[i].state = TP_STATE_ALLOCED;
			viimeksi_loytynyt = i;
			return i;
		}
		i = (i + 1) % MAX_PROCESSES;
	} while (i != viimeksi_loytynyt);

	return NO_PROCESS;
}

pid_t find_running_process(void)
{
	static pid_t viimeksi_loytynyt;
	pid_t i = viimeksi_loytynyt;
	do {
		i = (i + 1) % MAX_PROCESSES;
		if (processes[i].state == TP_STATE_RUNNING) {
			viimeksi_loytynyt = i;
			return i;
		}
	} while (i != viimeksi_loytynyt);

	return NO_PROCESS;
}

pid_t new_process(const void *code, size_t code_size, uint_t entry_offset, const void * stack, size_t stack_size, int user)
{
	pid_t pid;
	tid_t tid;
	uint_t prog_pages;

	if ((pid = alloc_process()) == NO_PROCESS) {
		return NO_PROCESS;
	}

	struct process * const process = processes + pid;

	// Uusi sivuhakemisto
	const uint_t phys_pd = build_new_pagedir(0);
	if (!phys_pd) {
		free_process(pid);
		return NO_PROCESS;
	}
	process->mem.phys_pd = phys_pd;

	// Tilaa ohjelmalle
	prog_pages = alloc_program_space(phys_pd, code_size, code, 1);
	if (!prog_pages) {
		free_process(pid);
		return NO_PROCESS;
	}
/*
	// Kopioidaan ohjelma
	page = prog_pages;
	code_ptr = code;
	while (copied_size + MEMORY_PAGE_SIZE < code_size) {
		ptr = temp_virt_page(0, phys_pd, page);
		if (!ptr) {
			free_process(pid);
			return NO_PROCESS;
		}
		memcpy(ptr, code_ptr, MEMORY_PAGE_SIZE);
		temp_virt_page(0, 0, 0);
		copied_size += MEMORY_PAGE_SIZE;
		code_ptr += MEMORY_PAGE_SIZE;
		++page;
	}
	ptr = temp_virt_page(0, phys_pd, page);
	memcpy(ptr, code_ptr, code_size - code_size);
	temp_virt_page(0, 0, 0);
*/
	// Luodaan aloittava säie
	tid = new_thread(pid, (entry_t)((uintptr_t)PAGE_TO_ADDR(prog_pages) + entry_offset), stack, stack_size, user);
	if (tid == NO_THREAD) {
		free_process(pid);
		return NO_PROCESS;
	}
	process->threads.tid0 = tid;
	process->vt_num = VT_KERN_LOG;
	process->state = TP_STATE_RUNNING;

	return pid;
}
