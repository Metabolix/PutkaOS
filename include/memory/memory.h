#ifndef _MEMORY_H
#define _MEMORY_H 1

#include <memory/swap.h>
#include <stdint.h>

/**
Mapping of the first 1M = 256 pages of memory (virtual and physical!)
0x00 - 0x00 : ( 1 page ) RESERVED
0x01 - 0x0f : (15 pages) free pages for use with temp_page_(enable|disable)
0x10 - 0x17 : ( 8 pages) physical pages allocation bitmap
0x18 - 0x1f : ( 8 pages) physical pages noswap bitmap
0x3d - 0x3d : ( 1 page ) temporary page directory for use in creating new
0x3e - 0x3e : ( 1 page ) temporary page table for use in creating new ones
0x3f - 0x3f : ( 1 page ) kernel page directory
0x40 - 0x7f : (64 pages) kernel page tables

Other virtual mappings
0x3ff - 0x3ff : ( 1 page ) active page directory
0x400 - 0x7ff : (1024 pages) active page tables
**/

#define TEMP_PAGES_START (0x01)
#define TEMP_PAGES_COUNT (0x0f)
#define PHYSICAL_PAGES_ALLOCATION_BITMAP (0x10)
#define PHYSICAL_PAGES_NOSWAP_BITMAP (0x18)
#define TEMPORARY_PAGE_DIRECTORY (0x3d)
#define TEMPORARY_PAGE_TABLE (0x3e)
#define KERNEL_PAGE_DIRECTORY (0x3f)
#define KERNEL_PAGE_TABLES (0x40)

//#define ACTIVE_PAGE_DIRECTORY (0x3ff)
//#define ACTIVE_PAGE_TABLES (0x400)

struct memory_info {
	uint32_t ram_pages, ram_pages_free;
	uint32_t swap_pages, swap_pages_free;
	uint32_t kernel_ram_pages; // no swap for kernel.
	uint32_t user_ram_pages, user_swap_pages;
};

typedef struct page_entry_t page_entry_t;
struct page_entry_t {
	unsigned
		present : 1,
		write : 1,
		user : 1,
		reserved : 9,
		pagenum : 20;
};

// #define USER_PAGE_PARAMS	7
// #define KERNEL_PAGE_PARAMS	3
#define NULL_PE ((page_entry_t){0})
#define USER_PE(page) ((page_entry_t){1,1,1,(0),(page)})
#define KERNEL_PE(page) ((page_entry_t){1,1,0,(0),(page)})
#define NEW_PE(page, user) (user ? USER_PE(page) : KERNEL_PE(page))

#define MEMORY_PAGE_SIZE (4096)
#define MEMORY_PE_COUNT (1024)

#define KERNEL_PDE_COUNT (64) // Kernel may use 64*1024*4096 = 256MB memory
#define KERNEL_PAGES (2 * 256) // 2M (ei kmalloc vaan itse kernel muistissa; suoraan virtuaalisesta fyysiseen; sis. pagedir) TODO: Kaivetaan nämä tiedot multibootista!
#define K_ALLOC_MAX_PAGE (256 * 256) // 256M (kmalloc)

#define STACK_PAGES_BEGIN (256 * 256)
#define STACK_PAGES_END (512 * 256)
#define STACK_PAGES_COUNT (STACK_PAGES_END - STACK_PAGES_BEGIN)

#define MAX_STACKS (256)
#define MAX_PAGES_PER_STACK (STACK_PAGES_COUNT / MAX_STACKS)
#define MAX_STACK_SIZE (1024*1024 - MEMORY_PAGE_SIZE)
/// Stack: [free-pages] [overflow-page] [stack-pages] [underflow-page]

#define U_ALLOC_PAGES_BEGIN STACK_PAGES_END
#define MEMORY_PAGES (4096 * 256) // 4G

#define PHYS_PAGE_BITMAP_LEN32 (MEMORY_PAGES / 32)

#define ADDR_TO_PAGE(x) ((uint_t)(x) >> 12)
#define PAGE_TO_ADDR(x) ((void*)((uint_t)(x) << 12))

#define STACK_TOP_PAGE(x) (STACK_PAGES_BEGIN + (((x) + 1) * MAX_PAGES_PER_STACK) - 2)

#define PD_NOSWAP_FLAG (1)
#define PT_NOSWAP_FLAG (1)

extern uint32_t ram_free(void);
extern uint32_t memory_free(void);

extern uint_t alloc_phys_page(int noswap);
extern void free_phys_page(uint_t block);

extern void * temp_phys_page(uint_t page, uint_t phys_page);
extern void * temp_virt_page(uint_t page, uint_t phys_pagedir, uint_t virt_page);
extern page_entry_t * temp_page_directory(uint_t phys_page);
extern page_entry_t * temp_page_table(uint_t phys_page);

extern void use_pagedir(uint_t phys_pagedir);

// ---

extern uint_t physical_page_of(uint_t virtual_page);
//extern void *physical_address_of(void *ptr);

//extern int mmap(uint_t real_page, uint_t virtual_page, int user);
//extern void unmap(uint_t virtual_page);
extern uint_t find_free_virtual_pages(uint_t phys_pagedir, uint_t count, int user);
extern uint_t alloc_virtual_pages(uint_t phys_pagedir, uint_t count, int user);
extern int map_virtual_page(uint_t phys_pagedir, uint_t virt_page, int noswap, int user);
extern void unmap_virtual_page(uint_t phys_pagedir, uint_t virt_page);

extern void pagedir_init(page_entry_t *addr);
extern void memory_init(uint_t max_mem);

extern void malloc_init(void);

extern uint_t build_new_pagedir(uint_t old_phys_pd);
extern uint_t alloc_program_space(uint_t phys_pagedir, uint_t size, const void *code, int user);
extern int resize_stack(uint_t phys_pagedir, int stack_num, uint_t size, int user);
extern void free_pagedir(uint_t phys_pagedir);
/*
extern uint_t alloc_physical_page();
extern uint_t alloc_virtual_page();
extern uint_t alloc_virtual_pages(uint_t count);
*/
#endif
