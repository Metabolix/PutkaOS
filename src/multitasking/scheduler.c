#include <multitasking/scheduler.h>
#include <multitasking/multitasking.h>
#include <multitasking/thread.h>
#include <multitasking/process.h>
#include <tss.h>

#include <string.h>

extern struct kernel_tasks kernel_tasks;

static void save_thread_state(void)
{
	if (!active_thread) {
		return;
	}
	memcpy(&active_thread->tss, &kernel_tasks.tss_for_active_thread, sizeof(struct tss));
}

static void load_thread_state(void)
{
	if (!active_thread) {
		return;
	}
	memcpy(&kernel_tasks.tss_for_active_thread, &active_thread->tss, sizeof(struct tss));
}

void scheduler(void)
{
	save_thread_state();
	select_next_thread();
	load_thread_state();
}

void select_next_thread(void)
{
	if (!threading_started) {
		return;
	}
	if (thread_count > 1 || !active_thread) {
		tid_t tid;
		pid_t pid;
		do {
			tid = find_running_thread();
		} while (tid == NO_THREAD);
		pid = threads[tid].pid;

		active_tid = tid;
		active_pid = pid;
	}

	active_thread = threads + active_tid;
	active_process = processes + active_pid;
}
