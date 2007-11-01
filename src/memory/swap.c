#include <memory/swap.h>
#include <stdint.h>

/**
* swap_in - haetaan sivu RAMiin
* @phys_pd: fyysinen sivu, jolla sivutaulu sijaitsee
* @virt_page: haettavan virtuaalisen sivun numero
*
* Palauttaa saadun fyysisen sivun
**/
uint_t swap_in(uint_t phys_pd, uint_t virt_page)
{
	// TODO: swap_in (phys_pd, virt_page)
	return 0;
}

/**
* swap_out - siirretään RAM-sivu swap-tilaan
* @phys_pd: fyysinen sivu, jolla sivutaulu sijaitsee
* @virt_page: poistettavan virtuaalisen sivun numero
*
* Palauttaa onnistuessaan 0
**/
int swap_out(uint_t phys_pd, uint_t virt_page)
{
	// TODO: swap_out (phys_pd, virt_page)
	return -1;
}

/**
* swap_free_page - vapautetaan yksi RAM-sivu siirtämällä se swap-tilaan
*
* Palauttaa vapautetun fyysisen sivun
**/
uint_t swap_free_page(void)
{
	return 0;
}
