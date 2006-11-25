#include <mem.h>
#include <screen.h>
#include <panic.h>
#include <memory.h>

void init_pde(int pde);

unsigned long * page_directory = (unsigned long *) 0x9C000;
unsigned long * page_table = (unsigned long *) 0x9D000;
unsigned long * memory_table = (unsigned long *) 0x10000; /* [is_free, is_protected, is_free, is_protected, ...] */

#define MAX_MEMORY ((page_table - memory_table)) /* one byte can point one kilobyte (because 4 bytes can point page, 4 kilobytes) */
int continue_block = 0;
int ram_count = 20 * 1024;
int block_count = 5 * 1024;


unsigned int get_bit(unsigned int num, unsigned char bit) {
        if(bit < 32)
                return (num) & (1 << bit);
        else
                return 0;
}

int get_cr0()
{
	int a;
	asm("mov %%cr0,%0":"=r"(a));
	return a;
}

void set_cr0(int cr0)
{
	asm("mov %0,%%cr0"::"r"(cr0));
}

void set_cr3(int cr3)
{
	asm("mov %0,%%cr3"::"r"(cr3));
}

unsigned int set_bit(unsigned int num, unsigned char bit, char value) {
        if(bit < 32) {
                if(value)
                        num |= (1 << bit);
                else
                        num &= ~(1 << bit);
        }
        return num;
}

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
	for(i = 0; i < pde_len; i++) {
		if(page_directory[i] & 1) { /* if this PDE exists */
			for(a = 0; a < pde_len; a++) {
				o = (page_table[i * pde_len + a] & ~2048) / block_size;
				if(!(page_table[i * pde_len + a] & 1)) {
					print("It was the ");
					print_hex(pde_len * i + a);
					print(" pte\n");
					return pde_len * i + a;
				} else if(!(get_bit(memory_table[o / 16], (o % 16) * 2) > 0)) {
						print("Page_table ");
						print_hex(i * pde_len + a);
						print(" has address ");
						print_hex(o);
						print("\n");
						return pde_len * i + a;
				}
			}
		}
	}
	/* we need to set up new page_table */
	for(i = 0; i < pde_len; i++) {
		if((i * pde_len) > MAX_MEMORY) { /* not enough memory */
			break;
		}
		if(!(page_directory[i] & 1)) {
			init_pde(i);
			print("It was the ");
			print_hex(pde_len * i + a);
			print(" pte2\n");

			return i * pde_len;
		}
	}
	panic("Out of PTEs");
	return 0;
}

void init_pde(int pde) {
	int i;
	int pde_pointer = pde_len * pde;
	for(i = 0; i < pde_len; i++)
		page_table[pde_pointer + i] = 2;
	page_directory[pde] = (((int)page_table + pde_pointer) | 3);
}

void * alloc_page() {
	int block = find_free_block();
	void * page = (void *)(block * block_size);
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
	return (void*)(block * block_size);
}

void free_real(void * pointer) {
	int block;
        block = ((int)pointer) / block_size;

        if(get_bit(memory_table[block / 16], (block % 16) * 2 + 1))
                panic("Trying to free protected memory!");
	

	if(block < continue_block)
                continue_block = block;

        memory_table[block / 16] = set_bit(memory_table[block/16], block % 16 * 2,0);
}
	

void free_page(void * pointer) {
	int block = ((int)pointer) / block_size;

	if(get_bit(memory_table[block / 16], (block % 16) * 2 + 1))
		panic("Trying to free protected memory!");
	
	/*for(i = 0; i < pde_len; i++) { * find correct PTE * /
		if(page_directory[i] & 1) { * if this PDE exists * /
			for(a = 0; a < pde_len; a++) {
				if((page_table[i * pde_len + a] & (0xFFFFFFFF - 4095)) == (long)pointer) {
					 pte = pde_len * i + a;
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

	for(a = 0; a < 512; a++, address += block_size) /* set up page_table, map 2 megas of ram */
		page_table[a] = address | 3; /* these memory locations exist */
	for(a = 512; a < block_count; a++, address += block_size)
		page_table[a] = address | 2;
	
	page_directory[0] = (int)page_table | 3; /* okay, we have one page_table (for now) */

	set_cr3((int)page_directory);  /* put that page directory address into CR3 */
	set_cr0(get_cr0() | 0x80000000); /* paging bit on */
	if(get_cr0() & 0x80000000)
		print("Memory paging is enabled!\n");

	print("We have ");
	print_hex(block_count);
	print(" blocks\n");
	

}
