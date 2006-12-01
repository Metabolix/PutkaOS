#include <memory.h>
#include <mem.h>
#include <panic.h>
#include <screen.h>


typedef struct memory_block {
	void * start;
	int len;
	void * next;
} memory_block_t;

typedef struct memory_entry {
	int pte;
	memory_block_t * mem;
} memory_entry_t;

#define memarea_entries 511

typedef struct memory_area { 
	memory_entry_t entries[511]; /* 511 */ 
	void * next;
} memory_area_t;

/*typedef struct list {
	void * pointer;
	list_t * next;
} list_t;*/

#define plist_memblocks 341 /* (4096-sizeof(void*)) / sizeof(memory_block) */

typedef struct page_list {
	memory_block_t memblock[plist_memblocks];
	void * next;
} page_list_t;

page_list_t * mem_pages = 0; 
memory_area_t * mem_area = 0;

extern long * page_directory;
extern long * page_table;
extern void init_pde(int p);
memory_block_t * get_memblock();
memory_entry_t * get_mementry();

/* Get area of free PTEs (and create them if need)  */
int get_free_pte_area(int size) {
        int a, i, find = 0;
        for(i = 0; i < pde_len; i++) {
                if(page_directory[i] & 1) { /* if this PDE exists */
                        for(a = 0; a < pde_len; a++) {
                                if(!(page_table[i * pde_len + a] & 1)) {
                                        if(++find == size)	/* if we found many enough */
						return pde_len * i + a;
                                }
				else
					find = 0;
                        }
                } else {
			init_pde(i);
			i--;
		}
        }
        panic("Out of PTEs");
        return 0;
}


void * alloc_pages(int size)
{
	int a = 0;
	/*int got = 0;*/
	/*memory_area_t * mem_ar = mem_area;*/
	int pte = get_free_pte_area(size); /* TODO: support areas which are not multiples of 4KB */

	int mem_got = size;
	memory_block_t * mem_block = get_memblock();


	

	mem_block->start = alloc_real();
	mem_block->len = 4096;
	mem_got--;

	while(mem_got > 0){ 
		mem_block->next = get_memblock();	/* switch to next block */
		mem_block = (memory_block_t*)mem_block->next;
		
		mem_block->start = alloc_real();	/* allocate */
		mem_block->len = 4096;
		
		page_table[pte + a] = (int)mem_block->start | 3; /* map address */

		mem_got -= 1; /* -= memblock_len */ 
		a++;
	}

	return (void*)(pte * 4096);
}


memory_block_t * get_memblock() { /* get area for memory*/
	int a;
	void * pointer;
	page_list_t * pages_p = mem_pages;

	if(pages_p == 0) {
		pages_p = alloc_page();
		memset(pages_p, 0, 4096);
	}

	while(1) {
		for(a = 0; a < plist_memblocks; a++) { /* loop through all blocks */
			pointer = (void *)pages_p->memblock;
			if(((memory_block_t *) pointer)[a].len == 0) {
				return &((memory_block_t*)(pointer))[a];
			}
		}
		if(pages_p->next == 0) {	/* use next page_list */
			pages_p->next = alloc_page();
			memset(pages_p->next, 0, 4096);
		}
		pages_p = pages_p->next;
	}
	return 0;
}

memory_entry_t * get_mementry() {
	int a;
	void * pointer;
	memory_area_t * mem_area_p = mem_area;

	if(mem_area == 0) {
                mem_area = (memory_area_t *) alloc_page();
		memset(mem_area, 0, 4096);
	}

	while(1) {
		for(a = 0; a < memarea_entries; a++) {
			pointer = mem_area_p->entries;
			if(((memory_entry_t*) pointer)[a].mem == 0) {
				return &((memory_entry_t*)(pointer))[a];
			}
		}
		if(mem_area_p->next == 0) {
			mem_area_p->next = alloc_page();
			memset(mem_area_p->next, 0, 4096);
		}
		mem_area_p = mem_area_p->next;
	}
	return 0;
}
