#include <multitasking/multitasking.h>
#include <multitasking/process.h>
#include <multitasking/thread.h>
#include <string.h>
#include <panic.h>
#include <debug.h>
#include <misc_asm.h>
#include <vt.h>
#include <gdt.h>
#include <idt.h>

extern void kmain2(void);
extern void switch_task(void);

int has_threading(void)
{
	return active_thread || 0;
}

int is_threading(void)
{
	return has_threading() && !is_in_irq_handler();
}

void threading_init(void)
{
	memset(processes, 0, sizeof(processes));
	memset(threads, 0, sizeof(threads));

	processes[0] = (struct process) {
		.mem = {
			.phys_pd = KERNEL_PAGE_DIRECTORY,
		},
		.uid = 0,
		.gid = 0,
		.state = TP_STATE_RUNNING,
		.vt_num = VT_KERN_LOG,
		.threads = {
			.tid0 = 0,
			.count = 0,
		},
	};
	process_count = 1;
	active_pid = 0;
	processes[0].threads.tid0 = active_tid = new_thread(0, kmain2, 0, 0, 0);
	processes[0].threads.count = 1;

	active_process = processes + active_pid;
	switch_thread();
}

void select_next_thread(void)
{
	if (!active_process) {
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

	if (!active_process->mem.phys_pd) {
		panic("PD puuttuu!\n");
	}
}
