#ifndef _MEMORY_VARS_H
#define _MEMORY_VARS_H 1

#include <memory/memory.h>

extern uint32_t * const phys_pages_alloced;
extern uint32_t * const phys_pages_noswap;

extern uint_t first_phys_pages_not_alloced;
extern struct memory_info memory;

#endif
