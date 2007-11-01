#ifndef _SWAP_H
#define _SWAP_H 1

#include <memory/memory.h>
#include <stdint.h>

extern uint_t swap_in(uint_t phys_pagedir, uint_t virt_page);
extern int swap_out(uint_t phys_pagedir, uint_t virt_page);
extern uint_t swap_free_page(void);

#endif
