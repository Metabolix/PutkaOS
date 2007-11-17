#include <multitasking/multitasking.h>
#include <multitasking/thread.h>
#include <multitasking/process.h>
#include <memory/memory.h>
#include <memory/kmalloc.h>
#include <stddef.h>
#include <string.h>
#include <panic.h>
#include <kprintf.h>
#include <vt.h>
#include <misc_asm.h>
#include <gdt.h>
#include <idt.h>

struct thread threads[MAX_THREADS];
size_t thread_count = 0;

tid_t active_tid = NO_THREAD;
struct thread * active_thread = 0;

tid_t threads_ended_arr[MAX_THREADS];
uint_t threads_ended = 0;

const struct tss kernel_space_tss = {
	.cs = KERNEL_CS_SEL,
	.ds = KERNEL_DS_SEL,
	.es = KERNEL_DS_SEL,
	.fs = KERNEL_DS_SEL,
	.gs = KERNEL_DS_SEL,
	.ss = KERNEL_DS_SEL,
	.ss0 = KERNEL_DS_SEL,
	.eflags = 0x0202,
	.iopb_offset = sizeof(struct tss),
};

const struct tss user_space_tss = {
	.cs = USER_CS_SEL,
	.ds = USER_DS_SEL,
	.es = USER_DS_SEL,
	.fs = USER_DS_SEL,
	.gs = USER_DS_SEL,
	.ss = USER_DS_SEL,
	.ss0 = KERNEL_DS_SEL,
	.eflags = 0x0202,
	.iopb_offset = sizeof(struct tss),
};

static void free_thread(tid_t tid);
void clean_threads(void);

void switch_thread(void)
{
	asm_int(IDT_SCHEDULER);
}

void kill_thread(tid_t tid)
{
	if (threads[tid].state == TP_STATE_ALLOCED) {
		threads_ended_arr[threads_ended] = tid;
		++threads_ended;
		--thread_count;
		return;
	}
	if (threads[tid].state == TP_STATE_ENDED) {
		return;
	}
	if (threads[tid].state == TP_STATE_FREE) {
		panic("kill_thread: state == FREE");
	}
	const pid_t pid = threads[tid].pid;

	if (tid == active_tid) {
		asm_cli();
	}
	threads[tid].state = TP_STATE_ENDED;
	threads_ended_arr[threads_ended] = tid;
	++threads_ended;
	--thread_count;

	if (tid == processes[pid].threads.tid0) {
		// TODO: kill_thread: jatkavatko prosessin muut säikeet, jos tid0 kuolee?
		kill_process(pid);
	}
	if (tid == active_tid) {
		// Pitää vaihtaa uusi säie
		switch_thread();
	}
}

static void free_thread(tid_t tid)
{
	if (threads[tid].state == TP_STATE_ALLOCED) {
		goto clearing;
	}
	if (threads[tid].state != TP_STATE_ENDED) {
		panic("free_thread: not ended!");
	}
	if (tid == active_tid) {
		panic("free_thread: thread active!");
	}
	const pid_t pid = threads[tid].pid;
	--processes[pid].threads.count;
	process_free_thread_num(pid, threads[tid].thread_of_proc);
clearing:
	memset(&threads[tid], 0, sizeof(threads[tid]));
	threads[tid].state = TP_STATE_FREE;
}

void clean_threads(void)
{
	while (threads_ended) {
		--threads_ended;
		free_thread(threads_ended_arr[threads_ended]);
	}
}

/**
* function thread_ending - isr_thread_ending siirtyy tänne
**/
void thread_ending(void)
{
	kprintf("thread_ending_func: active_tid %i\n", active_tid);
	if (active_tid == 0) {
		panic("Thread 0 ending!\n");
	}
	kill_thread(active_tid);
	for (;;) switch_thread();
}

tid_t alloc_thread(void)
{
	static tid_t viimeksi_loytynyt;
	clean_threads();
	tid_t i = viimeksi_loytynyt;
	do {
		if (threads[i].state == TP_STATE_FREE) {
			memset(threads + i, 0, sizeof(threads[i]));
			threads[i].state = TP_STATE_ALLOCED;
			viimeksi_loytynyt = i;
			return i;
		}
		i = (i + 1) % MAX_THREADS;
	} while (i != viimeksi_loytynyt);

	return NO_THREAD;
}

tid_t find_running_thread(void)
{
	static tid_t viimeksi_loytynyt = MAX_THREADS - 1;
	tid_t i = viimeksi_loytynyt;
	do {
		i = (i + 1) % MAX_THREADS;
		if (threads[i].state == TP_STATE_RUNNING) {
			if (processes[threads[i].pid].state == TP_STATE_RUNNING) {
				viimeksi_loytynyt = i;
				return i;
			}
		}
	} while (i != viimeksi_loytynyt);

	return NO_THREAD;
}

int prepare_thread(struct thread *thread, uint_t phys_pd, const entry_t entry, const void * stack, size_t stack_size, int user)
{
	const char *sptr;
	char *pptr;
	uint_t page;
	int offset;
	size_t whole_size = stack_size + sizeof(entry_t) + (user ? 0 : 4*3);
	if (stack_size < 0 || resize_stack(phys_pd, thread->thread_of_proc, whole_size, user)) {
		return -1;
	}
	page = STACK_TOP_PAGE(thread->thread_of_proc);
	sptr = stack;

	// Initial stack
	while (stack_size >= MEMORY_PAGE_SIZE) {
		stack_size -= MEMORY_PAGE_SIZE;
		if (!(pptr = temp_virt_page(0, phys_pd, page))) {
			kprintf("init_stack: temp_virt_page(0, %d, %d) == NULL\n", phys_pd, page);
			panic("init_stack: temp_virt_page -> NULL");
		}
		memcpy(pptr, sptr + stack_size, MEMORY_PAGE_SIZE);
		temp_virt_page(0, 0, 0);
		--page;
	}
	if (!(pptr = temp_virt_page(0, phys_pd, page))) {
		kprintf("init_stack: temp_virt_page(0, %d, %d) == NULL\n", phys_pd, page);
		panic("init_stack: temp_virt_page -> NULL");
	}
	offset = MEMORY_PAGE_SIZE;
	if (stack_size) {
		offset -= stack_size;
		memcpy(pptr + offset, sptr, stack_size);
	}

	// TSS
	memcpy(&thread->tss, (user ? &user_space_tss : &kernel_space_tss), sizeof(struct tss));

	// Stack for iret.
	if (user) {
		thread->irq_stack[5] = thread->tss.ss;
		thread->irq_stack[4] = (uint32_t) PAGE_TO_ADDR(page) + offset;
		thread->irq_stack[3] = thread->tss.eflags;
		thread->irq_stack[2] = thread->tss.cs;
		thread->irq_stack[1] = (uintptr_t) entry;
		thread->tss.esp0 = ((uintptr_t) &(thread->irq_stack)) + sizeof(thread->irq_stack);
		thread->tss.esp = (uintptr_t) &(thread->irq_stack[1]);
		thread->tss.ss0 =
		thread->tss.ss = KERNEL_DS_SEL;
		thread->tss.cs = KERNEL_CS_SEL;
	} else {
		// TODO: if (offset < 0), seuraavalle sivulle osa!
		offset -= 12;
		((uintptr_t *)(pptr + offset))[2] = thread->tss.eflags;
		((uintptr_t *)(pptr + offset))[1] = thread->tss.cs;
		((uintptr_t *)(pptr + offset))[0] = (uintptr_t) entry;
		thread->tss.esp = (uint32_t) PAGE_TO_ADDR(page) + offset;
	}
	thread->tss.cr3 = (uintptr_t) PAGE_TO_ADDR(phys_pd);
	thread->tss.eflags &= ~(1 << 9); // Interrupts disabled
	extern void irq_common_ret(void);
	thread->tss.eip = (uintptr_t) irq_common_ret;

	temp_virt_page(0, 0, 0);
	return 0;
}

tid_t new_thread(pid_t pid, entry_t entry, const void * stack, size_t stack_size, int user)
{
	tid_t tid;
	int i;

	if ((tid = alloc_thread()) == NO_THREAD) {
		return NO_THREAD;
	}
	if ((i = process_alloc_thread_num(pid)) < 0) {
		free_thread(tid);
		return NO_THREAD;
	}
	struct thread * const thread = threads + tid;
	struct process * const process = processes + pid;

	thread->pid = pid;
	thread->thread_of_proc = i;

	if (prepare_thread(thread, process->mem.phys_pd, entry, stack, stack_size, user) != 0) {
		free_thread(tid);
		return NO_THREAD;
	}

	thread->pid = pid;
	thread->state = TP_STATE_RUNNING;
	++thread_count;
	return tid;
}
