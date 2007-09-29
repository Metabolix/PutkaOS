#include <string.h>
#include <screen.h>
#include <panic.h>
#include <memory.h>
#include <bit.h>
#include <misc_asm.h>

void init_pde(unsigned int pde);

unsigned long * temporary_page_directory = (unsigned long *) 0x40000;
unsigned long * kernel_page_tables = (unsigned long *) 0x41000;

unsigned long * page_directory = (unsigned long *) 0x3FF000;
unsigned long * page_table = (unsigned long *) 0x400000;

/* page_table[0-4096] are mapped in page_directory[0-4] (for "normal memory"), page_table[4096-6144] are in page_directory[4-5] (for malloc) */

unsigned long * memory_table = (unsigned long *) 0x10000; /* [is_free, is_protected, is_free, is_protected, ...] */

unsigned int continue_block = 16;
//unsigned int ram_count = 20 * 1024;
unsigned int block_count = 0x8000; /* 0x8000 * 32 * 4096 = 4GB */

unsigned int pages_free;


/* Finds free block from physical ram */
int find_free_block(void)
{
	unsigned int i, j;

	i = continue_block;
	for (i = continue_block; i < block_count; i++) {
		if (memory_table[i] != 0xffffffff) {  /* All is-reserved bits */
			for(j = 0; j < sizeof(int) * 8; j++) {
				if (get_bit(memory_table[i], j) == 0) {
					/*  T�m� ei ole v�ltt�m�tt� hyv� idea... sen j�lkeen kun on 20 taskia,
					    joista jokainen vapauttaa muistia miten haluaa, nopeiten sivu _saattaa_
					    l�yty� kuitenkin alusta. Katotaan miten toimii :P
					continue_block = i; */
					return (i * 32) + j;
				}
			}
		}
		//kprintf("It wasn't the %u^th memory_table_entry\n", i);
	}
	panic("MEM: Out of memory!");
	return 0;
}

void reserve_block(unsigned int block)
{
	if(get_bit(memory_table[block / 32], block % 32) == 0)
	{
		pages_free--;
		memory_table[block / 32] = set_bit(memory_table[block / 32], block % 32, 1);

	}
}

void release_block(unsigned int block)
{
	if(get_bit(memory_table[block / 32], block % 32) == 0)
	{
		pages_free++;
		memory_table[block / 32] = set_bit(memory_table[block / 32], block % 32, 0);
	}
}

unsigned int memory_free(void) {
	return pages_free * 0x1000;
}

void * get_physical_address(void * addr)
{
	return (void *)(page_table[((((unsigned int)addr)>>22) * 1024) + ((((unsigned int)addr)>>12) & 1023)] & 0xFFFFF000);
}

/* Search a free page and allocates given real page to given virtual address. Create new PDE if needed */
int mmap(void * real, void * virtual_address, int type)
{
	unsigned int virtual_addr = (unsigned int)(virtual_address) & 0xFFFFF000;

	// Do we have page directory already?
	if(page_directory[((unsigned int)virtual_addr)>>22] == 0x0)
	{
		// No. Create one
		unsigned int dir_block = find_free_block();

		/* Out of memory */
		if(dir_block==0)
			return 1;

		// Set up new PDE
		page_directory[virtual_addr>>22] = (dir_block * MEMORY_BLOCK_SIZE) | KERNEL_PAGE_PARAMS;
		reserve_block(dir_block);

		// Write address of the new page table to the page table of memory window
		page_table[1024 + (virtual_addr>>22)] = (dir_block * MEMORY_BLOCK_SIZE) | KERNEL_PAGE_PARAMS;
		asm_set_cr3(asm_get_cr3());
	}
	page_table[((virtual_addr>>22) * 1024) + ((virtual_addr>>12) & 1023)] = (((unsigned int)real) & 0xFFFFF000) | type;
	asm_set_cr3(asm_get_cr3());
	return 0;
}

// Unmaps given virtual page
// Todo: Release the page table when all its entries are unmaped
void unmap(void * virtual_address)
{
	unsigned int virtual_addr = (unsigned int)(virtual_address) & 0xFFFFF000;

	page_table[((virtual_addr>>22) * 1024) + ((virtual_addr>>12) & 1023)] = 0x0;
	asm_set_cr3(asm_get_cr3());
}

// Searchs free area from virtual memory
void * find_area(unsigned int area_size, unsigned char memory_type)
{
	unsigned int curr_pde, curr_pte, area_start, area_found, last_pde;

	area_start=0;

	if(memory_type==KERNEL_PAGE_PARAMS)
	{
 		curr_pde=0;
		last_pde=KERNEL_PDE_COUNT;
	}
	else
	{
		curr_pde=KERNEL_PDE_COUNT;
		last_pde=1024;
	}

	for(area_found=0; area_found < area_size && curr_pde<last_pde; curr_pde++)
	{
		if(page_directory[curr_pde]==0x0)
		{
			area_found+=0x400000;
			if(area_start==0)
				area_start=curr_pde*0x400000;
		}
		else
			for(curr_pte=0; area_found < area_size && curr_pte<1024; curr_pte++)
			{
				if(page_table[curr_pde*1024+curr_pte] == 0x0)
				{
					area_found+=0x1000;
					if(area_start==0)
						area_start=curr_pde*0x400000 + curr_pte*0x1000;
				}
				else
				{
					area_found=0;
					area_start=0;
				}
			}
	}

	if(area_found==0)
		return 0;
	else
		return (void *) area_start;
}

void pagedir_init(unsigned long * addr)
{
	unsigned int a, address;
	for (a = 0, address=0x41000; a < KERNEL_PDE_COUNT; a++, address += MEMORY_BLOCK_SIZE)
		addr[a] = address | KERNEL_PAGE_PARAMS;
}

void memory_init(unsigned int max_mem)
{
	unsigned int a;
	unsigned int address = 0;

	/* Two MBs are reserved for kernel */
	pages_free = max_mem/4 - 512;

	/* We reserve first 2 megabytes for our kernel */
	for (a = 0; a < 16; a++) {
		memory_table[a] = 0xFFFFFFFF;
	}
	for (a = 16; a < block_count; a++) {
		memory_table[a] = 0x0;
	}

	// Kernel may use 256MB
	memset(kernel_page_tables, 0, MEMORY_BLOCK_SIZE * KERNEL_PDE_COUNT);

	/* Set up first page table, map 2 megas of ram */
	for (a = 0; a < 511; a++, address += MEMORY_BLOCK_SIZE) {
		kernel_page_tables[a] = address | KERNEL_PAGE_PARAMS;
	}

	/* Memory area 3FF000h - 400000h refers to page directory */
	kernel_page_tables[1023] = ((unsigned long) temporary_page_directory) | KERNEL_PAGE_PARAMS;

	/* Memory area 400000h - 800000h refers to current page tables */
	for (a = 0, address= (unsigned int) kernel_page_tables; a < KERNEL_PDE_COUNT; a++, address += MEMORY_BLOCK_SIZE) {
		kernel_page_tables[a + 1024] = address | KERNEL_PAGE_PARAMS;
	}

	memset(temporary_page_directory, 0, 4096);

	pagedir_init(temporary_page_directory);

	/* Directories set. Let's bring up paging */
	asm_set_cr3(temporary_page_directory);
	asm_set_cr0((void*)((unsigned long) asm_get_cr0() | 0x80000000));

	if ((unsigned long) asm_get_cr0() & 0x80000000) {
		print("Memory paging is enabled!\n");
	}

	kprintf("Memory management is enabled. Using %d bytes of memory\n", pages_free * 0x1000);
	//kprintf("MEM: Using %u kilobytes of %u available\n", memory, ram_count);
}
