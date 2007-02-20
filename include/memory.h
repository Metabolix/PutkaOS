#ifndef _MEMORY_H
#define _MEMORY_H

extern void * alloc_page(void);
extern void * alloc_real(void);
extern void free_real(void * pointer);
extern void free_page(void * pointer);
extern void memory_init(unsigned int memory);

extern void mmap(unsigned int real, unsigned int virtual_addr);
extern void unmmap(unsigned int virtual_addr);

#define MEMORY_BLOCK_SIZE  4096
#define MEMORY_PDE_LEN  1024

#define MAX_MEMORY (0x4000) /* We don't need more than 16 megabytes of memory */


#endif
