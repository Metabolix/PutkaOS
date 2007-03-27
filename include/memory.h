#ifndef _MEMORY_H
#define _MEMORY_H

extern int find_free_block(void);
extern void reserve_block(unsigned int block);
extern void release_block(unsigned int block);
extern void * get_physical_address(void * addr);

extern int mmap(void * real, void * virtual_address, int type);
extern void unmap(void * virtual_address);
extern void * find_area(unsigned int asize, unsigned char memory_type);

extern void pagedir_init(unsigned long *addr);
extern void memory_init(unsigned int max_mem);

extern unsigned int memory_free(void);

#define MEMORY_BLOCK_SIZE	4096
#define MEMORY_PDE_LEN		1024

// TE = Table entry (for later use..)
#define TE_PRESENT		1
#define TE_READWRITE		2
#define TE_USER			4

#define USER_PAGE_PARAMS	7
#define KERNEL_PAGE_PARAMS	3

#define MAX_MEMORY 		0x4000	/* We don't need more than 16 megabytes of memory */
#define KERNEL_PDE_COUNT	64	/* Kernel may use 64*1024*4096 = 256MB memory */

#endif
