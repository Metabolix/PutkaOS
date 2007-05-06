#include <thread.h>
#include <malloc.h>
#include <string.h>
#include <panic.h>
#include <screen.h>
#include <putkaos.h>

struct process_t processes[K_MAX_PROCESSES];
struct thread_t threads[K_MAX_THREADS];

process_id_t active_process = NO_PROCESS;
thread_id_t active_thread = NO_THREAD;

struct process_t * active_process_ptr = 0;
struct thread_t * active_thread_ptr = 0;
int threading_started = 0;
size_t num_processes = 0;
size_t num_threads = 0;

const struct regs_t initial_regs = {
	0x10, 0x10, 0x10, 0x10, 0x10, // gs, fs, es, ds, ss;
	0, 0, 0, 0, 0, 0, 0, 0, // edi, esi, ebp, esp, ebx, edx, ecx, eax;
	0, 0, // int_no, err_code;
	0, 0x08, 0x0202, // eip, cs, eflags;
};

/**
 * function kernel_idle_loop - hlt & jmp
**/
extern void kernel_idle_loop(void);

/**
 * function thread_ending - this is where "ret" leads from a thread entry
**/
void thread_ending(void)
{
	extern void process_ending(void);
	kprintf("active_thread %i\n", active_thread);
	if (active_thread == 0) {
		/* Idle thread... Jatketaan vain. xD */
		kernel_idle_loop();
	}
	kprintf("Thread %i ending...\n", active_thread);
	kprintf("num_threads = %d, --\n", num_threads);
	kprintf("processes[threads[active_thread].process].num_threads = %d, --\n", processes[threads[active_thread].process].num_threads); for(;;);
	--num_threads;
	--processes[threads[active_thread].process].num_threads;
	if (processes[threads[active_thread].process].num_threads == 0) {
		process_ending();
	} else if (active_thread == processes[threads[active_thread].process].main_thread) {
		process_ending();
	}

	threads[active_thread].process = 0;
	kfree((void*)threads[active_thread].stack);
	threads[active_thread].stack = 0;
	threads[active_thread].running = 0;
	// TODO: Lista loppuneista säikeistä, että voidaan vapauttaa niiden pinot aina välillä... Eli ei vapauteta täällä.
	panic("Thread ended - What do we do now?");
}

thread_id_t find_free_thread(void)
{
	static thread_id_t free_thread;
	thread_id_t alk_free_thread = free_thread;
	do {
		if (threads[free_thread].stack == 0) {
			alk_free_thread = free_thread;
			free_thread = (free_thread + 1) & (K_MAX_THREADS - 1); // % K_MAX_THREADS
			return alk_free_thread;
		}
		free_thread = (free_thread + 1) & (K_MAX_THREADS - 1); // % K_MAX_THREADS
	} while (free_thread != alk_free_thread);

	return NO_THREAD;
}

thread_id_t find_running_thread(void)
{
	static thread_id_t running_thread;
	thread_id_t alk_running_thread = running_thread;
	do {
		if (threads[running_thread].running) {
			alk_running_thread = running_thread;
			running_thread = (running_thread + 1) & (K_MAX_THREADS - 1);
			return alk_running_thread;
		}
		running_thread = (running_thread + 1) & (K_MAX_THREADS - 1);
	} while (alk_running_thread != running_thread);
	return NO_THREAD;
}

thread_id_t new_thread(t_entry entry, void * initial_stack, size_t initial_stack_size)
{
	thread_id_t thread;
	char *esp;
	thread = find_free_thread();
	if (thread == NO_THREAD) {
		return NO_THREAD;
	}
	threads[thread].stack = (uint_t) kmalloc(K_THREAD_STACK_SIZE);
	kprintf("stacki = %p, + %x = %p\n", threads[thread].stack, K_THREAD_STACK_SIZE, threads[thread].stack + K_THREAD_STACK_SIZE);
	if (threads[thread].stack == 0) {
		return NO_THREAD;
	}
	esp = (char*) (threads[thread].stack + K_THREAD_STACK_SIZE);

	esp -= initial_stack_size;
	memcpy(esp, initial_stack, initial_stack_size);
	if (initial_stack) kprintf("initial_stack = %p\n", *(void**)initial_stack);

	esp -= sizeof(t_entry);
	*(t_entry*)esp = thread_ending;

	threads[thread].esp = ((struct regs_t*)esp) - 1;
	memcpy(threads[thread].esp, &initial_regs, sizeof(struct regs_t));
	threads[thread].esp->eip = (uint_t)entry;
	threads[thread].esp->esp = (uint_t)&(threads[thread].esp->ss);
	threads[thread].esp->ebp = threads[thread].stack + K_THREAD_STACK_SIZE;
	threads[thread].ss = threads[thread].esp->ss;
	threads[thread].process = active_process;
	threads[thread].running = 1;

	++num_threads;
	return thread;
}

void kill_thread(thread_id_t thread)
{
	kfree((void*)threads[thread].stack);
	threads[thread].stack = 0;
}

void threading_init(void)
{
	memset(processes, 0, sizeof(processes));
	memset(threads, 0, sizeof(threads));
	if ((active_process = new_process(kernel_idle_loop, 0, 0))
	|| (active_thread = processes[active_process].main_thread)) {
		kprintf("threading_init: active_thread = %d, active_process = %d\n",
			active_thread, active_process);
		panic("threading_init: Threading has a problem!\n");
	}
	processes[active_process].vt_num = VT_KERN_LOG;
}

void start_threading(void)
{
	if (active_thread_ptr) {
		panic("start_threading: already started!\n");
	}
	asm_cli();
	active_process_ptr = processes + active_process;
	active_thread_ptr = threads + active_thread;

	extern void start_idle_thread(void *thread_ptr);
	start_idle_thread(active_thread_ptr);
}

void next_thread(void)
{
	if (!active_process_ptr || num_threads < 2) {
		return;
	}
	do { active_thread = find_running_thread(); } while (active_thread == 0);
	active_thread_ptr = threads + active_thread;
}
