#ifndef _MEMORY_H
#define _MEMORY_H

void * alloc_page();
void * alloc_real();
void free_real(void * pointer);
void free_page(void * pointer);
void init_memory(int memory_size);

#define MEMORY_BLOCK_SIZE  4096
#define MEMORY_PDE_LEN  1024


#endif
