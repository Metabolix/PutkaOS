#include <multitasking/multitasking.h>
#include <multitasking/thread.h>
#include <multitasking/process.h>
#include <memory/memory.h>
#include <memory/kmalloc.h>
#include <stddef.h>
#include <string.h>
#include <panic.h>
#include <screen.h>
#include <vt.h>
#include <misc_asm.h>
#include <irq.h>

struct thread threads[MAX_THREADS];
size_t thread_count = 0;

tid_t active_tid = NO_THREAD;
struct thread * active_thread = 0;

tid_t threads_ended_arr[MAX_PROCESSES];
uint_t threads_ended = 0;

const struct regs kernel_space_regs = {
	0x10, 0x10, 0x10, 0x10, 0x10, // gs, fs, es, ds, ss;
	0, 0, 0, 0, 0, 0, 0, 0, // edi, esi, ebp, esp, ebx, edx, ecx, eax;
	0, 0, // int_no, err_code;
	0, 0x08, 0x0202, // eip, cs, eflags;
};

const struct regs user_space_regs = {
	0x20, 0x20, 0x20, 0x20, 0x20, // gs, fs, es, ds, ss;
	0, 0, 0, 0, 0, 0, 0, 0, // edi, esi, ebp, esp, ebx, edx, ecx, eax;
	0, 0, // int_no, err_code;
	0, 0x18, 0x0202, // eip, cs, eflags;
};

static void free_thread(tid_t tid);
void clean_threads(void);

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

	threads[tid].state = TP_STATE_ENDED;
	threads_ended_arr[threads_ended] = tid;
	++threads_ended;
	--thread_count;

	if (tid == processes[pid].threads.tid0) {
		// TODO: kill_thread: jatkavatko prosessin muut säikeet, jos tid0 kuolee?
		kill_process(pid);
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
 * function thread_ending - this is where "ret" leads from a thread entry
**/
void thread_ending_func(void)
{
	kprintf("thread_ending_func: active_tid %i\n", active_tid);
	if (active_tid == 0) {
		panic("Thread 0 ending!\n");
	}
	kill_thread(active_tid);
	for (;;) asm_hlt(); // TODO: Säikeen loppuun vaihto!
}
static const entry_t thread_ending = thread_ending_func;

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
	static tid_t viimeksi_loytynyt;
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

void * init_stack(uint_t phys_pd, int thread_of_proc, entry_t entry, const void * stack, size_t stack_size, int user)
{
	const char *sptr;
	char *pptr;
	struct regs *rptr;
	uint_t page;
	uint_t offset;
	size_t whole_size = stack_size + sizeof(entry_t) + sizeof(struct regs);
	if (stack_size < 0 || resize_stack(phys_pd, thread_of_proc, whole_size, user)) {
		return 0;
	}
	page = STACK_TOP_PAGE(thread_of_proc);
	sptr = stack;

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
	offset -= sizeof(entry_t);
	memcpy(pptr + offset, &thread_ending, sizeof(entry_t));

	rptr = ((struct regs*) (pptr + offset)) - 1;
	memcpy(rptr, (user ? &user_space_regs : &kernel_space_regs), sizeof(struct regs));
	rptr->eip = (uint32_t) entry;
	// "kutsuvan funktion" (thread_ending) EBP
	rptr->ebp = (uint32_t) PAGE_TO_ADDR(STACK_TOP_PAGE(thread_of_proc) + 1);
	// ESP ennen pushad-käskyä, vaikka prosessori ei tätä käytäkään! Vaan näyttääpä aidolta.
	rptr->esp = (uint32_t) PAGE_TO_ADDR(page) + offset + offsetof(struct regs, ss);
	offset -= sizeof(struct regs);

	temp_virt_page(0, 0, 0);
	return (char*)PAGE_TO_ADDR(page) + offset;
}

tid_t new_thread(pid_t pid, entry_t entry, const void * stack, size_t stack_size, int user)
{
	tid_t tid;
	const struct regs * const regs = (user ? &user_space_regs : &kernel_space_regs);
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

	thread->esp = init_stack(process->mem.phys_pd, thread->thread_of_proc, entry, stack, stack_size, user);
	if (!thread->esp) {
		free_thread(tid);
		return NO_THREAD;
	}

	thread->ss = regs->ss;
	thread->pid = pid;
	thread->state = TP_STATE_RUNNING;
	++thread_count;
	return tid;
#if 0
	char *esp;
	esp = (char*) (threads[tid].stack + THREAD_STACSIZE);

	esp -= stack_size;
	memcpy(esp, stack, stack_size);
	if (stack) kprintf("stack = %p\n", *(void**)stack);

	esp -= sizeof(entry_t);
	*(entry_t*)esp = thread_ending;

	threads[tid].esp = ((struct regs*)esp) - 1;
	memcpy(threads[tid].esp, regs, sizeof(struct regs));
	threads[tid].esp->eip = (uint_t)entry;
	threads[tid].esp->esp = (uint_t)&(threads[tid].esp->ss);
	threads[tid].esp->ebp = threads[tid].stack + THREAD_STACSIZE;
	threads[tid].ss = threads[tid].esp->ss;
	threads[tid].pid = active_pid;
	threads[tid].state = TP_STATE_RUNNING;

	++thread_count;
#endif
}
