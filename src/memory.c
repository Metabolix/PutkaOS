#include <mem.h>
#include <screen.h>
#include <panic.h>
#include <memory.h>
#include <bit.h>

void init_pde(unsigned int pde);

unsigned long * page_directory = (unsigned long *) 0x9C000;
unsigned long * page_table = (unsigned long *) 0x9D000; /* page_table[0-4096] are mapped in page_directory[0-4] (for "normal memory"), page_table[4096-6144] are in page_directory[4-5] (for malloc) */
unsigned long * memory_table = (unsigned long *) 0x10000; /* [is_free, is_protected, is_free, is_protected, ...] */


unsigned int continue_block = 0;
unsigned int ram_count = 20 * 1024;
unsigned int block_count = 5 * 1024;

unsigned int get_cr0();
void set_cr0(int cr0);
void set_cr3(void*);
__asm__(
"get_cr0:\n"
"    movl %cr0,%eax\n"
"    ret\n"
"set_cr0:\n"
"    movl 4(%esp),%eax\n"
"    movl %eax,%cr0\n"
"    ret\n"
"set_cr3:\n"
"    movl 4(%esp),%eax\n"
"    movl %eax,%cr3\n"
"    ret\n"
);


/* finds free block from physical ram */
int find_free_block()
{
	unsigned int i, j;

	i = continue_block;
	for (i = continue_block; i < block_count; i++) {
		if ((memory_table[i] & 0xaaaaaaaa) != 0xaaaaaaaa) {  /* All is-reserved bits */
			for(j = 0; j < sizeof(int) * 4; j++) {
				if (get_bit(memory_table[i], 2 * j) == 0) {
					continue_block = i;
					return (i * 16) + j;
				}
			}
		}
		//kprintf("It wasn't the %u^th memory_table_entry\n", i);
	}
	panic("Out of memory!");
	return 0;
}


/* find free PTE (and creates possibly new PDE) */
int find_free_pte()
{
	unsigned int i, j, o;
	for (i = 0; i < MEMORY_PDE_LEN; i++) {
		/* Does it exist? */
		if ((page_directory[i] & 1) == 0) {
			continue;
		}
		for (j = 0; j < MEMORY_PDE_LEN; j++) {
			o = (page_table[i * MEMORY_PDE_LEN + j] & ~2048) / MEMORY_BLOCK_SIZE;
			if ((page_table[i * MEMORY_PDE_LEN + j] & 1)) {
				kprintf("It was the %#010x pte\n", MEMORY_PDE_LEN * i + j);
				return MEMORY_PDE_LEN * i + j;
			/* If this is already pointed, but memory_table-entry which points to it isn't allocated */
			} else if (get_bit(memory_table[o >> 4], (o & 0x0f) << 1) == 0) {
				kprintf("Page_table %#010x has address %p\n", (i * MEMORY_PDE_LEN + j), o);
				return MEMORY_PDE_LEN * i + j;
			}
		}
	}
	/* we need to set up new page_table */
	for(i = 0; i < MEMORY_PDE_LEN; i++) {
		if ((i * MEMORY_PDE_LEN) > ram_count) { /* not enough memory */
			break;
		}
		if ((page_directory[i] & 1) == 0) {
			init_pde(i);
			kprintf("It was the %#010x pte2.\n", MEMORY_PDE_LEN * i);

			return i * MEMORY_PDE_LEN;
		}
	}
	panic("Out of PTEs");
	return 0;
}

void mmap(unsigned int real, unsigned int virtual_addr) {
	page_table[virtual_addr >> 12] = real | 3;
}

void unmmap(unsigned int virtual_addr) {
	page_table[virtual_addr >> 12] = 2;
}


void init_pde(unsigned int pde) {
	unsigned int i;
	unsigned int pde_pointer = MEMORY_PDE_LEN * pde;
	for(i = 0; i < MEMORY_PDE_LEN; i++) {
		page_table[pde_pointer + i] = 2;
	}
	page_directory[pde] = (((unsigned int)page_table + pde_pointer) | 3);
}

void * alloc_page() {
	unsigned int block = find_free_block();
	void * page = (void *)(block * MEMORY_BLOCK_SIZE);
	unsigned int pte = block; /* find_free_pte(); */

	kprintf("Find_free_block returned %p\n", block);
	memory_table[block >> 4] = set_bit(memory_table[block >> 4], (block & 0x0f) << 1, 1);
	page_table[pte] = (unsigned long)page | 3;

	set_cr3(page_directory);

	return (void*)(pte * 4096);
}

void * alloc_real() {
	unsigned int block = find_free_block();
	memory_table[block >> 4] = set_bit(memory_table[block >> 4], (block & 0x0f) << 1, 1);
	return (void*)(block * MEMORY_BLOCK_SIZE);
}

void free_real(void * pointer) {
	unsigned int block;
	block = ((unsigned int)pointer) / MEMORY_BLOCK_SIZE;

	if (get_bit(memory_table[block >> 4], ((block & 0x0f) << 1) + 1)) {
		panic("Trying to free protected memory!");
	}

	if (block < continue_block) {
		continue_block = block;
	}

	/* memory_table[block / 16] = set_bit(memory_table[block >> 4], (block & 0x0f) << 1, 0); */
	memory_table[block >> 4] = set_bit(memory_table[block >> 4], (block & 0x0f) << 1, 0);
}


void free_page(void * pointer) {
	int block = ((int)pointer) / MEMORY_BLOCK_SIZE;

	if (get_bit(memory_table[block >> 4], ((block & 0x0f) << 1) + 1)) {
		panic("Trying to free protected memory!");
	}

#if 0
	for (i = 0; i < MEMORY_PDE_LEN; i++) { /* find correct PTE */
		if(page_directory[i] & 1) { /* if this PDE exists */
			for(a = 0; a < MEMORY_PDE_LEN; a++) {
				if((page_table[i * MEMORY_PDE_LEN + a] & (0xFFFFFFFF - 4095)) == (long)pointer) {
					 pte = MEMORY_PDE_LEN * i + a;
					 break;
				}
			}
		}
	}
	if (pte != -1) {
		page_table[pte] = 2;
	}
#endif
	/* it doesn't exist anymore, unmmap */
	if(block >> 2 < MAX_MEMORY)
		page_table[block] &= ~1;
 

	if(block < continue_block)
		continue_block = block;

	/* mark as free */
	/* memory_table[block / 16] = set_bit(memory_table[block >> 4], (block & 0x0f) << 1, 0); */
	memory_table[block >> 4] = set_bit(memory_table[block >> 4], (block & 0x0f) << 1, 0);
	set_cr3(page_directory);

}

void init_memory(unsigned int memory) {
	unsigned int a, i;
	unsigned int address = 0;

	ram_count = memory;
	if (ram_count > MAX_MEMORY) {
		ram_count = MAX_MEMORY;
	}

	block_count = ram_count / 4;

	/* we reserve first 2 megabytes for our kernel */
	for (a = 0; a < 32; a++) {
		memory_table[a] = 0xFFFFFFFF;
	}
	for (a = 32; a < (block_count / 32); a++) {
		memory_table[a] = 0x0;
	}

	/* set up page_table, map 2 megas of ram */
	for (a = 0; a < 512; a++, address += MEMORY_BLOCK_SIZE) {
		/* these memory locations exist */
		page_table[a] = address | 3;
	}
	for (a = 512; a < block_count; a++, address += MEMORY_BLOCK_SIZE) {
		page_table[a] = address | 2;
		if(a % 1024 == 0) {
			kprintf("&Page_table[%d] is %x\n", a, &page_table[a]);
		}
	}

	for(a = 4096; a < 6144; a++) {
		page_table[a] = 0;
	}

	/* page directories for malloc */
	page_directory[4] = (unsigned int)&page_table[4096] | 3;
	page_directory[5] = (unsigned int)&page_table[5120] | 3;

	/* page directories 0, and maybe 1,2,3 (if there is enough memory) */
	page_directory[0] = (unsigned int)page_table | 3;
	for(a = 0, i = 0; a < block_count / MEMORY_PDE_LEN; a++, i += MEMORY_PDE_LEN) {
		kprintf("Page_directory[%d] is %x\n", a, (unsigned int)page_table + i * sizeof(int));
		page_directory[a] = (((unsigned int) page_table) + i * sizeof(int)) | 3;
	}

	/* set base page directory */
	set_cr3(page_directory);

	/* enable memory paging */
	set_cr0(get_cr0() | 0x80000000);
	if (get_cr0() & 0x80000000) {
		print("Memory paging is enabled!\n");
	}

	kprintf("MEM: We have %u kilobytes of memory", memory);
	if (memory > ram_count) {
		kprintf(", but we use only %u kilobytes of it", ram_count);
	}
	putch('\n');
}
