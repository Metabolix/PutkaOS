#ifndef _MEMORY_H
#define _MEMORY_H

void * alloc_page();
void * alloc_real();
void free_real(void * pointer);
void free_page(void * pointer);
void init_memory(int memory_size);

#define block_size  4096
#define pde_len  1024


#endif
