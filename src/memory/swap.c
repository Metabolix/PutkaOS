#include <memory/swap.h>
#include <stdint.h>

/**
* swap_in - haetaan sivu RAMiin
* @phys_pagedir: fyysinen sivu, jolla sivutaulu sijaitsee
* @virt_page: haettavan virtuaalisen sivun numero
*
* Palauttaa saadun fyysisen sivun
**/
uint_t swap_in(uint_t phys_pagedir, uint_t virt_page)
{
	// TODO: swap_in (phys_pagedir, virt_page)
	return 0;
}

uint_t swap_free_page(void)
{
	return 0;
}
