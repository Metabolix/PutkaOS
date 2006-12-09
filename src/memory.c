#include <mem.h>
#include <screen.h>
#include <panic.h>
#include <memory.h>
#include <bit.h>

void init_pde(int pde);

unsigned long * page_directory = (unsigned long *) 0x9C000;
unsigned long * page_table = (unsigned long *) 0x9D000;
unsigned long * memory_table = (unsigned long *) 0x10000; /* [is_free, is_protected, is_free, is_protected, ...] */

#define MAX_MEMORY (0x4000) /* I guess that we don't need more than 16 megabytes of memory (at the moment) */
int continue_block = 0;
int ram_count = 20 * 1024;
int block_count = 5 * 1024;

int get_cr0();
void set_cr0(int cr0);
void set_cr3(int cr3);
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


int find_free_block() { /* finds free block from physical ram */
	int i = 0;
	int a;
	if(continue_block != 0)
		i = continue_block;

	for(; i < block_count; i++) {
		if((memory_table[i] & 0xaaaaaaaa) != 0xaaaaaaaa) {  /* All is-reserved bits */
			for(a = 0; a < sizeof(int) * 4; a++) {
				if(get_bit(memory_table[i], a * 2) == 0) {
						continue_block = i;
						return i * 16 + a;
				}
			}
		}
		print("It wasn't the ");
		print_hex(i);
		print(":th memory_table_entry\n");

	}
	panic("Out of memory");
	return 0; /* Otherwise gcc will throw warning */
}

int find_free_pte() { /* find free PTE (and creates possibly new PDE) */
	int a, i, o;
	for(i = 0; i < MEMORY_PDE_LEN; i++) {
		if(page_directory[i] & 1) { /* if this PDE exists */
			for(a = 0; a < MEMORY_PDE_LEN; a++) {
				o = (page_table[i * MEMORY_PDE_LEN + a] & ~2048) / MEMORY_BLOCK_SIZE;
				if(!(page_table[i * MEMORY_PDE_LEN + a] & 1)) {
					print("It was the ");
					print_hex(MEMORY_PDE_LEN * i + a);
					print(" pte\n");
					return MEMORY_PDE_LEN * i + a;
				/* If this is already pointed, but memory_table-entry which points to it isn't allocated */
				} else if(!(get_bit(memory_table[o / 16], (o % 16) * 2) > 0)) {
						print("Page_table ");
						print_hex(i * MEMORY_PDE_LEN + a);
						print(" has address ");
						print_hex(o);
						print("\n");
						return MEMORY_PDE_LEN * i + a;
				}
			}
		}
	}
	/* we need to set up new page_table */
	for(i = 0; i < MEMORY_PDE_LEN; i++) {
		if((i * MEMORY_PDE_LEN) > MAX_MEMORY) { /* not enough memory */
			break;
		}
		if(!(page_directory[i] & 1)) {
			init_pde(i);
			kprintf("It was the %#010x pte2.\n", MEMORY_PDE_LEN * i);

			return i * MEMORY_PDE_LEN;
		}
	}
	panic("Out of PTEs");
	return 0;
}

void init_pde(int pde) {
	int i;
	int pde_pointer = MEMORY_PDE_LEN * pde;
	for(i = 0; i < MEMORY_PDE_LEN; i++)
		page_table[pde_pointer + i] = 2;
	page_directory[pde] = (((int)page_table + pde_pointer) | 3);
}

void * alloc_page() {
	int block = find_free_block();
	void * page = (void *)(block * MEMORY_BLOCK_SIZE);
	int pte = find_free_pte();

	print("Find_free_block returned ");
	print_hex(block);
	print("\n");
	memory_table[(int)(block / 16)] = set_bit(memory_table[block / 16], (block % 16) * 2, 1);
	page_table[pte] = (long) page | 3;

	set_cr3((int)page_directory);

	return (void*)(pte * 4096);
}

void * alloc_real() {
	int block = find_free_block();
	memory_table[(int)(block / 16)] = set_bit(memory_table[block / 16], (block % 16) * 2, 1);
	return (void*)(block * MEMORY_BLOCK_SIZE);
}

void free_real(void * pointer) {
	int block;
        block = ((int)pointer) / MEMORY_BLOCK_SIZE;

        if(get_bit(memory_table[block / 16], (block % 16) * 2 + 1))
                panic("Trying to free protected memory!");


	if(block < continue_block)
                continue_block = block;

        memory_table[block / 16] = set_bit(memory_table[block/16], block % 16 * 2,0);
}


void free_page(void * pointer) {
	int block = ((int)pointer) / MEMORY_BLOCK_SIZE;

	if(get_bit(memory_table[block / 16], (block % 16) * 2 + 1))
		panic("Trying to free protected memory!");

	/*for(i = 0; i < MEMORY_PDE_LEN; i++) { * find correct PTE * /
		if(page_directory[i] & 1) { * if this PDE exists * /
			for(a = 0; a < MEMORY_PDE_LEN; a++) {
				if((page_table[i * MEMORY_PDE_LEN + a] & (0xFFFFFFFF - 4095)) == (long)pointer) {
					 pte = MEMORY_PDE_LEN * i + a;
					 break;
				}
			}
		}
	}
	if(pte != -1)
		page_table[pte] = 2;*/

	if(block < continue_block)
		continue_block = block;

	memory_table[block / 16] = set_bit(memory_table[block/16], block % 16 * 2,0); /* mark as free */
	set_cr3((int)page_directory);
}

void init_memory(int memory) {
	int a;
	int address = 0;

	if(memory >  MAX_MEMORY) /* set limit */
		ram_count = MAX_MEMORY;
	else
		ram_count = memory;

	block_count = ram_count / 4;

	for(a = 0; a < 32; a++)
		memory_table[a] = 0xFFFFFFFF; /* we reserve first 2 megabytes for our kernel */
	for(a = 32; a < block_count / 32; a++)
		memory_table[a] = 0x0;

	for(a = 0; a < 512; a++, address += MEMORY_BLOCK_SIZE) /* set up page_table, map 2 megas of ram */
		page_table[a] = address | 3; /* these memory locations exist */
	for(a = 512; a < block_count; a++, address += MEMORY_BLOCK_SIZE)
		page_table[a] = address | 2;

	page_directory[0] = (int)page_table | 3; /* okay, we have one page_table (for now) */

	set_cr3((int)page_directory);  /* put that page directory address into CR3 */
	set_cr0(get_cr0() | 0x80000000); /* paging bit on */
	if(get_cr0() & 0x80000000)
		print("Memory paging is enabled!\n");

	kprintf("MEM: We have %u kilobytes of memory", memory);
	if(memory > ram_count) {
		kprintf(", but we use only %u kilobytes of it", ram_count);
	}
	putch('\n');


}
