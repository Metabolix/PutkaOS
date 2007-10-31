#include <multitasking/multitasking.h>
#include <multitasking/process.h>
#include <multitasking/thread.h>
#include <string.h>
#include <panic.h>
#include <misc_asm.h>
#include <vt.h>

extern void kmain2(void);
extern void start_threading(void);

void threading_init(void)
{
	memset(processes, 0, sizeof(processes));
	memset(threads, 0, sizeof(threads));
	/*
	threads[0] = (struct thread) {
		.esp = ,
		.ss = 0x10,
		.pid = 0,
		.state = TP_STATE_RUNNING,
		.thread_of_proc = 0,
	};*/
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
	active_pid = 0;
	processes[0].threads.tid0 = active_tid = new_thread(0, kmain2, 0, 0, 0);
	processes[0].threads.count = 1;

	asm_cli();
	active_process = processes + active_pid;
	active_thread = threads + active_tid;
	start_threading();
	/*
	if ((active_pid = new_process(kernel_idle_loop, 0, 0, 0, 0))
	|| (active_tid = processes[active_pid].main_thread)) {
		kprintf("threading_init: active_tid = %d, active_pid = %d\n",
			active_tid, active_pid);
		panic("threading_init: Threading has a problem!\n");
	}
	processes[active_pid].vt_num = VT_KERN_LOG;
	*/
}
#if 0
void start_threading(void)
{
	if (active_thread) {
		panic("start_threading: already started!\n");
	}
	asm_cli();
	active_process = processes + active_pid;
	active_thread = threads + active_tid;

	extern void start_idle_thread(void *thread_ptr);
	start_idle_thread(active_thread);
}
#endif

void next_thread(void)
{
	if (!active_process || thread_count < 2) {
		return;
	}
	do { active_tid = find_running_thread(); } while (active_tid == 0);
	active_thread = threads + active_tid;
	active_pid = active_thread->pid;
	active_process = processes + active_pid;
	if (!active_process->mem.phys_pd) {
		panic("PD puuttuu!\n");
	}
	use_pagedir(active_process->mem.phys_pd);
}

int has_threading(void)
{
	return active_thread || 0;
}

int is_threading(void)
{
	return has_threading() && !is_in_irq_handler() && !is_in_exception_handler();
}
