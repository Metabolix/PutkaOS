#include <thread.h>
#include <malloc.h>
#include <string.h>
#include <panic.h>
#include <screen.h>

struct process_t processes[K_MAX_PROCESSES] = {{0}};
struct thread_t threads[K_MAX_THREADS] = {{0}};

process_id_t active_process = NO_PROCESS;
thread_id_t active_thread = NO_THREAD;

struct process_t * active_process_ptr = 0;
struct thread_t * active_thread_ptr = 0;

thread_id_t kernel_idle_process = NO_PROCESS;
thread_id_t kernel_idle_thread = NO_THREAD;

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

const char *started_idle_loop = "Started idle loop!\n";
void kernel_idle_loop(void);
__asm__(
"kernel_idle_loop:\n"
"    movl started_idle_loop, %eax\n"
"    pushl %eax\n"
"    call print\n"
"    addl $4, %esp\n"
"kernel_idle_loop_loop:\n"
"    hlt\n"
"    jmp kernel_idle_loop_loop\n"
);

/**
 * function thread_ending - this is where "ret" leads from a thread entry
**/
void thread_ending(void)
{
	extern void process_ending(void);
	kprintf("active_thread %i, kernel_idle_thread %i\n", active_thread, kernel_idle_thread);
	if (active_thread == kernel_idle_thread) {
		// TODO: Tunge sille sen oikea entrypointti takaisin. :P
		panic("Thread 0 (kernel idle thread) is ending!");
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
	// TODO: Siirry seuraavaan säikeeseen
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
	if (threads[thread].stack == 0) {
		return NO_THREAD;
	}
	esp = (char*) (threads[thread].stack + K_THREAD_STACK_SIZE);

	esp -= initial_stack_size;
	memcpy(esp, initial_stack, initial_stack_size);

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
	kernel_idle_process = new_process(kernel_idle_loop, 0, 0);
	kernel_idle_thread = processes[kernel_idle_process].main_thread;
	if (kernel_idle_process || kernel_idle_thread) {
		kprintf("kernel_idle_thread = %d\nkernel_idle_process = %d\n", kernel_idle_thread, kernel_idle_process);
		panic("Threading has a problem!\n");
	}
	processes[kernel_idle_process].vt_num = VT_KERN_LOG;
}

void start_threading(void)
{
	static int started; if (started) return; started = 1;

	asm("cli");
	active_process = kernel_idle_process;
	active_thread = kernel_idle_thread;

	active_process_ptr = processes + active_process;
	active_thread_ptr = threads + active_thread;

	extern void start_idle_thread();
	start_idle_thread();
}

void next_thread(void)
{
	if (active_thread == NO_THREAD || num_threads < 2) {
		return;
	}
	active_thread = find_running_thread();
	active_thread_ptr = threads + active_thread;
}
