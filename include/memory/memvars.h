#ifndef _MEMORY_VARS_H
#define _MEMORY_VARS_H 1

#include <memory/memory.h>

extern uint32_t * const phys_pages_alloced;
extern uint32_t * const phys_pages_noswap;

extern uint_t first_phys_pages_not_alloced;

extern struct memory_info memory;

static page_entry_t * const kernel_page_directory = (page_entry_t *) (KERNEL_PAGE_DIRECTORY * MEMORY_PAGE_SIZE);
static page_entry_t * const kernel_page_tables = (page_entry_t *) (KERNEL_PAGE_TABLES * MEMORY_PAGE_SIZE);

#endif
