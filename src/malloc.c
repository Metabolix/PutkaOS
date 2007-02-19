#include <memory.h>
#include <mem.h>
#include <panic.h>
#include <screen.h>



typedef struct _area {
	void * start;
	size_t size;
} area;

typedef struct _malloc_mem {
	area free_memory[255];
	area alloc_memory[255];

	void * next;  /* next malloc_mem */
	int free;  /* how many free entries in free_memory */
	int alloc; /* how many free entries in alloc_memory */
	int reserved; /* what should we do with this? ;) */
} malloc_mem;

typedef struct _mem_maps {  /* this would be the first thing which needs cleanup */
	int pte[1023];   /* we start at max_memory + 1*/
	void * next;
} mem_maps;

mem_maps * mem_map_first;
malloc_mem * malloc_mem_first;

int pages_mapped; /* pages mapped from beginning of the pages_table[4096] */

/* init_malloc_map - inits one malloc memory entry 
 *
 * @mem: pointer to the entry
 */

void init_malloc_map(malloc_mem * mem) {
	memset(malloc_mem_first, 0, sizeof(malloc_mem));
	
	malloc_mem_first->free=255;
	malloc_mem_first->alloc=255;
}

/* malloc_init - inits things */
 
void malloc_init() {
	malloc_mem_first = (malloc_mem*)alloc_page();

	pages_mapped = -1;
	
	init_malloc_map(malloc_mem_first);
	malloc_mem_first->free--;
	malloc_mem_first->free_memory[0].start = (void*)0x1000000;	/* first 2 megabytes are reserved */
	malloc_mem_first->free_memory[0].size = 0x800000;
}

/* mkmallocmem - find place for memory and marks it as allocated (doesn't allocate memory!)
 *
 * @size: size of the area to allocate
 *
 * returns area which was allocated */

area mkmallocmem(size_t size) { /* make space for memarea */
	malloc_mem * mmem_free_p = malloc_mem_first;
	malloc_mem * mmem_alloc_p = malloc_mem_first;
	int a,b;
	int cycles = 0;
	area ar = {(void*)0,0}; /* for nothing... */

	while(!mmem_alloc_p->alloc) { /* find new alloc thing */
		if(mmem_alloc_p->next) { 
			mmem_alloc_p = (malloc_mem *) mmem_alloc_p->next;
			cycles++;
		} else {
			mmem_alloc_p->next = (malloc_mem *) alloc_page();
			init_malloc_map(mmem_alloc_p->next);
			mmem_alloc_p = (malloc_mem *)mmem_alloc_p->next;
			cycles++;
			break;
		}
	}
	
	while(mmem_free_p) { /* find big enough free thing */
		for(a = 0; a < 255; a++) {
			if(mmem_free_p->free_memory[a].size >= size) { /* we can "take" memory from this entry */
				for(b = 0; b < 255; b++) {
					if(!mmem_alloc_p->alloc_memory[b].size) { /* free alloc entry */
						mmem_alloc_p->alloc--;
						mmem_alloc_p->alloc_memory[b].size = size;
						mmem_alloc_p->alloc_memory[b].start = mmem_free_p->free_memory[a].start;

						mmem_free_p->free_memory[a].start += size; /* move the free area forward */
						mmem_free_p->free_memory[a].size -= size;

						//kprintf("mmem_alloc_p->alloc=%d, mmem_alloc_p->alloc_memory[%d].size == %x .start = %x, mmem_free_p->free_memory[%d].size = %x, .start=%x\n", mmem_alloc_p->alloc, b, mmem_alloc_p->alloc_memory[b].size, (unsigned int) mmem_alloc_p->alloc_memory[b].start, a, mmem_free_p->free_memory[a].size, (unsigned int) mmem_free_p->free_memory[a].start);


						return mmem_alloc_p->alloc_memory[b];
					}
				}


			}
		}
		mmem_free_p = (malloc_mem *)mmem_free_p->next;
	}
	panic("OOM 8)\n");
	return ar;			
}

area destroymallocmem(void * start) {
	malloc_mem * mmem_free_p = malloc_mem_first;
	malloc_mem * mmem_alloc_p = malloc_mem_first;
	int a;
	int break_loop = 0;
	int cycles = 0;
	area ar = {(void*)0, 0};
	int combine = 0;
	area * to_combine = (area *)0;

	while(mmem_alloc_p) { /* look for start */
		for(a = 0; a < 255; a++) {
			if((unsigned int)mmem_alloc_p->alloc_memory[a].start == (unsigned int)start) {
				ar = mmem_alloc_p->alloc_memory[a];
				mmem_alloc_p->alloc++;
				mmem_alloc_p->alloc_memory[a].size = 0;
				break_loop = 1;
				break;
			}
		}
		if(break_loop)
			break;
		mmem_alloc_p = mmem_alloc_p->next;
		cycles++;
	}

	if(!mmem_alloc_p) {
		kprintf("Couldn't find thing to free\n");
		return ar;
	}

	/*kprintf("We had to do %d cycles\n", cycles);*/
	
	while(mmem_free_p) {
		for(a = 0; a < 255; a++) {
			if(mmem_free_p->free_memory[a].size)
				//kprintf("mmem_free_p->free_memory[%d].size = %x, .start=%x ar.start==%x start==%x ar.size==%x\n", a, mmem_free_p->free_memory[a].size, (unsigned int) mmem_free_p->free_memory[a].start, (unsigned int)ar.start, (unsigned int)start, ar.size);
			if((unsigned int)mmem_free_p->free_memory[a].start == ((unsigned int)start + ar.size)) {
				if(!(unsigned int)to_combine) {
					/*kprintf("Combined area to area %x:%x\n", mmem_free_p->free_memory[a].start, mmem_free_p->free_memory[a].size);*/
					mmem_free_p->free_memory[a].start -= ar.size;
					to_combine = &mmem_free_p->free_memory[a];
					combine = 0;
				} else {
					if(combine) {
						/*kprintf("Combined area to area %x:%x\n", to_combine->start, to_combine->size);*/
						to_combine->start -= mmem_free_p->free_memory[a].size;
						mmem_free_p->free_memory[a].size = 0;
						mmem_free_p->free++;
						
						return ar;
						/*break_loop = 1;
						break;*/
					}
				}
			}
			if(((unsigned int)mmem_free_p->free_memory[a].start + mmem_free_p->free_memory[a].size) == (unsigned int) start) {
				if(!(unsigned int)to_combine) {
					/*kprintf("Combined area to area %x:%x\n", mmem_free_p->free_memory[a].start, mmem_free_p->free_memory[a].size);*/
					mmem_free_p->free_memory[a].size += ar.size;
					to_combine = &mmem_free_p->free_memory[a];
					combine = 1;
				} else {
					if(!combine) {
						/*kprintf("Combined area to area %x:%x\n", to_combine->start, to_combine->size);*/
						to_combine->size += mmem_free_p->free_memory[a].size;
						mmem_free_p->free_memory[a].size = 0;
						mmem_free_p->free++;
						
						return ar;
						/*break_loop = 1;
						break;*/
					}
				}
			}
		}
		mmem_free_p = (malloc_mem *)mmem_free_p->next;
	}
	if(to_combine)
		return ar;
	/*kprintf("MEM: Couldn't combine area to any existing area\n");*/
	
	mmem_free_p = malloc_mem_first;
	while(mmem_free_p) {
		if(mmem_free_p->free) {
			for(a = 0; a < 255; a++) {
				if(mmem_free_p->free_memory[a].size == 0) {
					mmem_free_p->free--;
					mmem_free_p->free_memory[a] = ar;
					return ar;
				}
			}
		}
		mmem_free_p = (malloc_mem*)mmem_free_p->next;
		if((unsigned int)mmem_free_p == 0)
			mmem_free_p = (malloc_mem *)alloc_page();
	}
	panic("MEM: Couldn't mark free area");
	return ar;
}
	

/* kmalloc - our malloc function 
 *
 * @size: size of the area to allocate
 */

void * kmalloc(size_t size) {
	area memarea = mkmallocmem(size);
	int page_last = ((((unsigned int)memarea.start + memarea.size - 0x1000000) & ~0xFFF) >> 12) ;
	int a = 0;
	int cur_page = pages_mapped + 1;

	//kprintf("Page_first %d, page_last %d, pages_mapped %d\n", page_first, page_last, pages_mapped);
	if(page_last >= pages_mapped) {
		for(a = memarea.size; a > 0; a -= 4096, cur_page++) {
			mmap((unsigned int)alloc_real(), (cur_page + 0x1000) << 12);
			/*kprintf("Mapped a page at %x\n", (cur_page + 0x1000) << 12);
			kprintf("We will return %x\n", memarea.start);*/
		}
		pages_mapped = page_last;
	}
	return memarea.start;
}

void kfree(void * pointer) {
	malloc_mem * mmem_alloc_p = malloc_mem_first;
	int temp;
	int a;
	int last_alloc = 0;
	extern unsigned int * page_table;

	destroymallocmem(pointer);
	
	while(mmem_alloc_p) {
		if(mmem_alloc_p->alloc != 255) {
			for(a = 0; a < 255; a++) {
				if(mmem_alloc_p->alloc_memory[a].size != 0) {
					temp = (((unsigned int)mmem_alloc_p->alloc_memory[a].start + mmem_alloc_p->alloc_memory[a].size - 0x1000000) & ~0xFFF) >> 12;
					if(temp > last_alloc) {
						last_alloc = temp;
					}
				}
			}
		}
		mmem_alloc_p = (malloc_mem*)mmem_alloc_p->next;
	}
	if(pages_mapped < 0)
		return;

	if(last_alloc - 1 <= pages_mapped + 1) {
		for(a = last_alloc; a <= pages_mapped; a++ ) {
			/*kprintf("Going to free %p\n", (0x1000 + a) << 12);*/
			free_real((void*)((page_table[0x1000 + a]) & ~0xFFF));
			unmmap(0x1000000 + 0x1000 * a);
			/*kprintf("Freed a page\n");*/
		}
	}
	pages_mapped = last_alloc - 1;
}
