#ifndef _MEMORY_H
#define _MEMORY_H

extern void * alloc_page();
extern void * alloc_real();
extern void free_real(void * pointer);
extern void free_page(void * pointer);
extern void init_memory(int memory_size);

#define MEMORY_BLOCK_SIZE  4096
#define MEMORY_PDE_LEN  1024


#endif
