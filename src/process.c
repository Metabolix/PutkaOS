#include <thread.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <panic.h>

void process_ending(void)
{
	//panic("Jumalauta, ei kai prosessi voi loppua?!\n");
	int tid;
	for(tid = 0; tid < K_MAX_THREADS && active_process_ptr->num_threads; tid++) {
		if(threads[tid].process == active_process) {
			kill_thread(tid);
			active_process_ptr->num_threads--;
		}
	}

}

void create_user_page_tables(process_id_t pid)
{
	int a;
	int address = 0;
	processes[pid].pd =(int**) kmalloc(MEMORY_BLOCK_SIZE); /* TODO: use function which gives us one page */
	processes[pid].pd[0] =(int*) kmalloc(MEMORY_BLOCK_SIZE);
	processes[pid].pd[64] = (int*) kmalloc(MEMORY_BLOCK_SIZE);
	processes[pid].pd[65] = (int*) kmalloc(MEMORY_BLOCK_SIZE);

	memset(processes[pid].pd[64], 0, MEMORY_BLOCK_SIZE);
	memset(processes[pid].pd[65], 0, MEMORY_BLOCK_SIZE);
	

	for(a = 0; a < 511; a++, address += MEMORY_BLOCK_SIZE) {
		processes[pid].pd[0][a] = address | KERNEL_PAGE_PARAMS;
	}
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
		if (processes[i].state == pstate_running) {
			return i;
		}
	}
	return NO_THREAD;
}

process_id_t new_process(entry_t entry, void * initial_stack, size_t initial_stack_size, int user, int psize)
{
	process_id_t process;
	extern unsigned long * temporary_page_directory;
	
	process = find_free_process();
	
	if (process == NO_PROCESS) {
		return NO_PROCESS;
	}
	if(user)
		entry = (entry_t)(64*1024*4096 + ((int)entry & 0xFFF));

	processes[process].main_thread = new_thread(entry, initial_stack, initial_stack_size);
	if (processes[process].main_thread == NO_THREAD) {
		return NO_PROCESS;
	}
	if(user) { 
		int a;
		int p = (int)get_physical_address((void*)((int)entry)) & ~0xFFF;
		int stack = (int)get_physical_address((void*)(threads[processes[process].main_thread].stack & ~0xFFF));
		int stack_offset =  threads[processes[process].main_thread].stack & 0xFFF;
		int stack_size = K_THREAD_STACK_SIZE;
		int esp_offset = (int)threads[processes[process].main_thread].esp - (int)threads[processes[process].main_thread].stack;

		threads[processes[process].main_thread].stack = 65*1024*4096 + stack_offset;
		threads[processes[process].main_thread].esp = (struct regs_t*)((int)threads[processes[process].main_thread].stack + esp_offset);
		
		create_user_page_tables(process);
		for(a = 0; psize > 0; a++, psize-=MEMORY_BLOCK_SIZE) {
			processes[process].pd[64][a] =  (p + a * 0x1000) | USER_PAGE_PARAMS;
		}
		
		stack_size++;	
		for(a = 0; stack_size > 0; a++, stack_size-=MEMORY_BLOCK_SIZE) {
			processes[process].pd[65][a] = (stack + a * 0x1000) | USER_PAGE_PARAMS;
		}
		
		processes[process].pd[0] = (int*)((int)get_physical_address(processes[process].pd[0]) | KERNEL_PAGE_PARAMS);
		processes[process].pd[64] = (int*)((int)get_physical_address(processes[process].pd[64]) | USER_PAGE_PARAMS);
		processes[process].pd[65] = (int*)((int)get_physical_address(processes[process].pd[65]) | USER_PAGE_PARAMS);
		processes[process].pd = get_physical_address(processes[process].pd);
	} else {
		processes[process].pd = (int**)temporary_page_directory;
	}

	threads[processes[process].main_thread].process = process;
	processes[process].num_threads = 1;
	processes[process].state = pstate_running;
	
	return process;
}
