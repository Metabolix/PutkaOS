#include <memory/memory.h>
#include <memory/memvars.h>
#include <string.h>
#include <screen.h>
#include <panic.h>
#include <bit.h>
#include <stdint.h>
#include <misc_asm.h>

static page_entry_t * const cur_virt_pd = (page_entry_t *) ((KERNEL_PAGES - 1) * MEMORY_PAGE_SIZE);
static page_entry_t * const cur_virt_pt = (page_entry_t *) (KERNEL_PAGES * MEMORY_PAGE_SIZE);

/**
* ram_free - kertoo vapaan keskusmuistin määrän
**/
uint32_t ram_free(void)
{
	return memory.ram_pages_free * MEMORY_PAGE_SIZE;
}

/**
* memory_free - kertoo vapaan muistin määrän
**/
uint32_t memory_free(void)
{
	uint64_t mem = 0;
	mem += memory.ram_pages_free;
	mem += memory.swap_pages_free;
	mem *= MEMORY_PAGE_SIZE;
	if (mem > UINT32_MAX) {
		return UINT32_MAX;
	}
	return mem;
}

/**
* use_pagedir - vaihtaa käytettävää sivutaulua
* @phys_pagedir: fyysinen sivu, jota käytetään
**/
void use_pagedir(uint_t phys_pagedir)
{
	asm_set_cr3(PAGE_TO_ADDR(phys_pagedir));
}

/**
* alloc_phys_page - varaa fyysisen muistisivun
* @noswap: 1, jos sivua ei saa swapata kovalevylle
**/
uint_t alloc_phys_page(int noswap)
{
	uint_t i, j;
	uint_t page;

	if (!memory.ram_pages_free) {
		const uint_t page = swap_free_page();
		if (page) {
			i = page / 32;
			j = page % 32;
			goto loytyi;
		}
		panic("MEM: Out of memory!");
		return 0;
	}

	i = first_phys_pages_not_alloced;
	for (i = first_phys_pages_not_alloced; i < PHYS_PAGE_BITMAP_LEN32; i++) {
		if (phys_pages_alloced[i] == 0xffffffff) {
			continue;
		}
		for (j = 0; j < 32; j++) {
			if (phys_pages_alloced[i] & (1 << j)) {
				continue;
			}
			// TODO: first_phys_pages_not_alloced: sahausta vai silmukkaa?
			first_phys_pages_not_alloced = i;
			page = (i * 32) + j;
			goto loytyi;
		}
	}
	panic("MEM: Out of RAM and failed counting it!");
	return 0;

loytyi:;
	if (get_bit(phys_pages_alloced[i], j) == 0) {
		memory.ram_pages_free--;
		phys_pages_alloced[i] = set_bit(phys_pages_alloced[i], j, 1);
	}
	phys_pages_noswap[i] = set_bit(phys_pages_noswap[i], j, noswap);
	return page;
}

/**
* free_phys_page - vapauttaa fyysisen muistisivun
* @page: vapautettavan sivun numero
**/
void free_phys_page(uint_t page)
{
	const uint_t pos = page / 32, bitnum = page & 31;
	if (get_bit(phys_pages_alloced[pos], bitnum)) {
		memory.ram_pages_free++;
		phys_pages_alloced[pos] = set_bit(phys_pages_alloced[pos], bitnum, 0);
		if (first_phys_pages_not_alloced > pos) {
			first_phys_pages_not_alloced = pos;
		}
	}
}

/**
* temp_phys_page - väliaikainen käyttökelpoinen osoitin fyysiseen sivuun
* @page: virtuaalisivun numero (ei oikea sivunumero vaan tämän funktion asia)
* @phys_page: fyysinen sivu, johon virtuaalinen sivu pannaan osoittamaan, tai 0 poistoa varten
**/
void * temp_phys_page(uint_t page, uint_t phys_page)
{
	static uint32_t used[1 + (TEMP_PAGES_COUNT - 1) / 32];
	uint_t used_n, used_b, skip_check = 1;
	if (page < TEMP_PAGES_COUNT) {
		used_n = page / 32;
		used_b = page % 32;
		page += TEMP_PAGES_START;
	} else if (page == TEMPORARY_PAGE_DIRECTORY || page == TEMPORARY_PAGE_TABLE) {
		used_n = 0;
		used_b = 32;
	} else {
		panic("temp_phys_page abuse!\n");
		return 0;
	}
	void *address = PAGE_TO_ADDR(page);
	if (phys_page == 0) {
		cur_virt_pt[page].pagenum = page;
		asm_invlpg(address);
		if (!skip_check) {
			used[used_n] &= ~(1 << used_b);
		}
		return 0;
	}
	if (!skip_check) {
		if (used[used_n] & (1 << used_b)) {
			panic("temp_phys_page IN USE!\n");
			return 0;
		}
		used[used_n] |= (1 << used_b);
	}
	cur_virt_pt[page].pagenum = phys_page;
	asm_invlpg(address);
	return address;
}

/**
* temp_page_directory_or_table - väliaikainen sivuhakemisto tai -taulu paikalleen
* @phys_page: fyysinen sivu, jolla sivutaulu sijaitsee
* @table: 0, jos sivuhakemisto, 1, jos sivutaulu
**/
static page_entry_t * temp_page_directory_or_table(uint_t phys_page, int table)
{
	static uint_t active_phys_page[2];
	static page_entry_t *ptr[2];
	const uint_t pagenum[2] = {TEMPORARY_PAGE_DIRECTORY, TEMPORARY_PAGE_TABLE};

	if (!phys_page) {
		return 0;
	}
	if (active_phys_page[table] == phys_page) {
		return ptr[table];
	}
	active_phys_page[table] = phys_page;
	ptr[table] = temp_phys_page(pagenum[table], phys_page);
	return ptr[table];
}

/**
* temp_page_directory - väliaikainen sivuhakemisto paikalleen
* @phys_page: fyysinen sivu, jolla sivuhakemisto sijaitsee
**/
page_entry_t * temp_page_directory(uint_t phys_page)
{
	return temp_page_directory_or_table(phys_page, 0);
}

/**
* temp_page_table - väliaikainen sivutaulu paikalleen
* @phys_page: fyysinen sivu, jolla sivutaulu sijaitsee
**/
page_entry_t * temp_page_table(uint_t phys_page)
{
	return temp_page_directory_or_table(phys_page, 1);
}

/**
* temp_virt_page - väliaikainen käyttökelpoinen osoitin toisen taulun sivuun
* @page: virtuaalisivun numero (ei oikea sivunumero vaan tämän funktion asia)
* @phys_pagedir: fyysinen sivu, jolla sivutaulu sijaitsee, tai 0 poistoa varten
* @virt_page: haettavan virtuaalisen sivun numero tai 0 poistoa varten
**/
void * temp_virt_page(uint_t page, uint_t phys_pagedir, uint_t virt_page)
{
	page_entry_t *pd, *pt;
	uint_t new_location;

	if (!phys_pagedir || !virt_page) {
		return temp_phys_page(page, 0);
	}

	uint_t pde = virt_page / MEMORY_PE_COUNT;
	uint_t pte = virt_page % MEMORY_PE_COUNT;

	if (!(pd = temp_page_directory(phys_pagedir)) || !pd[pde].pagenum) {
		return 0;
	}
	if (!pd[pde].present) {
		new_location = swap_in(phys_pagedir, pd[pde].pagenum);
		if (!new_location) {
			return 0;
		}
		pd[pde].pagenum = new_location;
		pd[pde].present = 1;
	}
	if (!(pt = temp_page_table(pd[pde].pagenum)) || !pt[pte].pagenum) {
		return 0;
	}
	if (!pt[pte].present) {
		new_location = swap_in(phys_pagedir, pt[pte].pagenum);
		if (!new_location) {
			return 0;
		}
		pt[pte].pagenum = new_location;
		pt[pte].present = 1;
	}
	return temp_phys_page(page, pt[pte].pagenum);
}

/**
* find_free_virtual_pages - etsitään vapaita virtuaalisia sivuja
* @phys_pagedir: käytettävän sivuhakemiston fyysinen sivu (tai 0)
* @count: haluttujen sivujen määrä
* @user: sivuja kernelille (0) vai käyttäjälle (1)
**/
uint_t find_free_virtual_pages(uint_t phys_pagedir, uint_t count, int user)
{
	uint_t pde, pte, pde_end;
	uint_t found_page, found;
	page_entry_t *pd, *pt;

	if (user) {
		pde = KERNEL_PDE_COUNT;
		pde_end = MEMORY_PE_COUNT;
	} else {
		pde = 0;
		pde_end = KERNEL_PDE_COUNT;
		if (count >= memory.ram_pages - memory.kernel_ram_pages) {
			kprintf("%d >= %d - %d\n", count, memory.ram_pages, memory.kernel_ram_pages);
			return 0;
		}
		phys_pagedir = 0; // Kaikilla on samat kernelsivut
	}
	const uint_t phys_pd = phys_pagedir;

	if (phys_pd) {
		pd = temp_page_directory(phys_pd);
	} else {
		pd = cur_virt_pd;
	}
	for (found = 0; found < count && pde < pde_end; pde++) {
		if (pd[pde].pagenum == 0) {
			if (!found) {
				found_page = pde * MEMORY_PE_COUNT;
			}
			found += MEMORY_PE_COUNT;
			continue;
		}
		if (phys_pd) {
			pt = temp_page_table(pd[pde].pagenum);
		} else {
			pt = cur_virt_pt + (pde * MEMORY_PE_COUNT);
		}
		for (pte = (pde ? 0 : 1); found < count && pte < MEMORY_PE_COUNT; pte++) {
			if (pt[pte].pagenum != 0) {
				found = 0;
				continue;
			}
			if (!found) {
				found_page = pde * MEMORY_PE_COUNT + pte;
			}
			found++;
		}
	}

	if (found < count) {
		return 0;
	}

	return found_page;
}

/**
* alloc_virtual_pages - varataan virtuaalisia sivuja muuhun kuin aktiiviseen tauluun
* @phys_pagedir: käytettävän sivuhakemiston fyysinen sivu
* @count: haluttujen sivujen määrä
* @user: sivuja kernelille (0) vai käyttäjälle (1)
**/
uint_t alloc_virtual_pages(uint_t phys_pagedir, uint_t count, int user)
{
	uint_t first, page;

	if (!user) {
		phys_pagedir = 0; // Kaikilla on samat kernelsivut
	}

	first = find_free_virtual_pages(phys_pagedir, count, user);

	if (!first) {
		return 0;
	}

	for (page = first; page < first + count; ++page) {
		if (map_virtual_page(phys_pagedir, first, 0, user)) {
			goto free_pages_on_error;
		}
	}

	return first;

free_pages_on_error:
	while (page > first) {
		--page;
		unmap_virtual_page(phys_pagedir, page);
	}
	return 0;
}

/**
* map_virtual_page - osoitetaan virtuaalinen sivu jonnekin fyysiseen muistiin
* @phys_pagedir: käytettävän sivuhakemiston fyysinen sivu
* @virt_page: sivu, joka pitää osoittaa
* @noswap: pitääkö sivu pitää aina RAMissa?
* @user: sivuja kernelille (0) vai käyttäjälle (1)
**/
int map_virtual_page(uint_t phys_pagedir, uint_t virt_page, int noswap, int user)
{
	uint_t phys_page;
	uint_t pde, pte;
	page_entry_t *pd, *pt;

	pde = virt_page / MEMORY_PE_COUNT;
	pte = virt_page % MEMORY_PE_COUNT;

	if (pde >= KERNEL_PDE_COUNT && phys_pagedir) {
		goto external;
	}
	goto current;

current:
	if (cur_virt_pd[pde].pagenum == 0) {
		if (!(phys_page = alloc_phys_page(PT_NOSWAP_FLAG))) {
			return -1;
		}
		cur_virt_pd[pde] = KERNEL_PE(phys_page);
		//cur_virt_pt[KERNEL_PAGES + pde] = KERNEL_PE(phys_page);
		asm_flush_cr3();
	}
	phys_page = alloc_phys_page(noswap);
	if (!phys_page) {
		return -2;
	}
	cur_virt_pt[virt_page] = NEW_PE(phys_page, user);
	asm_flush_cr3();
	return 0;

external:
	pd = temp_page_directory(phys_pagedir);
	if (pd[pde].pagenum == 0) {
		if (!(phys_page = alloc_phys_page(PT_NOSWAP_FLAG))) {
			return -1;
		}
		pd[pde] = KERNEL_PE(phys_page);
	}
	pt = temp_page_table(pd[pde].pagenum);
	phys_page = alloc_phys_page(noswap);
	if (!phys_page) {
		return -2;
	}
	pt[pte] = NEW_PE(phys_page, user);
	asm_flush_cr3();
	return 0;
}

/**
* unmap_virtual_page - poistetaan virtuaalisen sivun osoitus
* @phys_pagedir: käytettävän sivuhakemiston fyysinen sivu
* @virt_page: sivu, joka pitää osoittaa
**/
void unmap_virtual_page(uint_t phys_pagedir, uint_t virt_page)
{
	uint_t new_location;
	uint_t pde, pte;
	page_entry_t *pd, *pt;

	pde = virt_page / MEMORY_PE_COUNT;
	pte = virt_page % MEMORY_PE_COUNT;

	if (pde >= KERNEL_PDE_COUNT && phys_pagedir) {
		goto external;
	}
	goto current;

current:
	if (cur_virt_pd[pde].pagenum == 0) {
		panic("unmap_virtual_page: (cur_virt_pd[pde].pagenum == 0)!");
	}
	if (!cur_virt_pd[pde].present) {
		new_location = swap_in(phys_pagedir, cur_virt_pd[pde].pagenum);
		if (!new_location) {
			panic("unmap_virtual_page (1)");
		}
		cur_virt_pd[pde].pagenum = new_location;
		cur_virt_pd[pde].present = 1;
	}

	if (cur_virt_pt[virt_page].pagenum == 0) {
		panic("unmap_virtual_page: (cur_virt_pt[virt_page].pagenum == 0)!");
	}
	if (!cur_virt_pt[virt_page].present) {
		new_location = swap_in(phys_pagedir, cur_virt_pt[virt_page].pagenum);
		if (!new_location) {
			panic("unmap_virtual_page (2)");
		}
		cur_virt_pt[virt_page].pagenum = new_location;
		cur_virt_pt[virt_page].present = 1;
	}
	free_phys_page(cur_virt_pt[virt_page].pagenum);
	cur_virt_pt[virt_page] = NULL_PE;
	asm_flush_cr3();
	return;

external:
	pd = temp_page_directory(phys_pagedir);
	if (pd[pde].pagenum == 0) {
		panic("unmap_virtual_page: (pd[pde].pagenum == 0)!");
	}
	if (!pd[pde].present) {
		new_location = swap_in(phys_pagedir, pd[pde].pagenum);
		if (!new_location) {
			panic("unmap_virtual_page (3)");
		}
		pd[pde].pagenum = new_location;
		pd[pde].present = 1;
	}

	pt = temp_page_directory(pd[pde].pagenum);
	if (pt[pte].pagenum == 0) {
		panic("unmap_virtual_page: (pt[pte].pagenum == 0)!");
	}
	if (!pt[pte].present) {
		new_location = swap_in(phys_pagedir, pt[pte].pagenum);
		if (!new_location) {
			panic("unmap_virtual_page (4)");
		}
		pt[pte].pagenum = new_location;
		pt[pte].present = 1;
	}
	pt[pte] = NULL_PE;
	asm_flush_cr3();
	return;
}

/**
* build_new_pagedir - luodaan sivutaulu uutta prosessia varten
**/
uint_t build_new_pagedir(uint_t old_phys_pd)
{
	uint_t new_phys_pd;
	page_entry_t *new_pd, *old_pd, *new_pt, *old_pt;
	void *old_page, *new_page;
	uint_t pde, pte;

	if (!(new_phys_pd = alloc_phys_page(PD_NOSWAP_FLAG))) {
		return 0;
	}
	new_pd = temp_phys_page(0, new_phys_pd);

	// Kopioidaan
	memset(new_pd, 0, MEMORY_PAGE_SIZE);
	memcpy(new_pd, kernel_page_directory, KERNEL_PDE_COUNT * sizeof(page_entry_t));

	if (!old_phys_pd) {
		return new_phys_pd;
	}
	old_pd = temp_phys_page(1, old_phys_pd);
	for (pde = KERNEL_PDE_COUNT; pde < MEMORY_PE_COUNT; ++pde) {
		if (old_pd[pde].pagenum == 0) {
			continue;
		}
		new_pd[pde] = old_pd[pde];
		new_pd[pde].pagenum = alloc_phys_page(PT_NOSWAP_FLAG);
		if (!new_pd[pde].pagenum) {
			panic("build_new_pagedir: (!new_pd[pde].pagenum) == muisti loppui!");
		}
		new_pt = temp_phys_page(2, new_pd[pde].pagenum);
		old_pt = temp_phys_page(3, old_pd[pde].pagenum);
		for (pte = 0; pte < MEMORY_PE_COUNT; ++pte) {
			if (old_pt[pte].pagenum == 0) {
				continue;
			}
			new_pt[pte] = old_pt[pte];
			new_pt[pte].pagenum = alloc_phys_page(0);
			if (!new_pt[pte].pagenum) {
				panic("build_new_pagedir: (!new_pt[pte].pagenum) == muisti loppui!");
			}
			new_page = temp_phys_page(4, new_pt[pde].pagenum);
			old_page = temp_phys_page(5, old_pt[pde].pagenum);
			memcpy(new_page, old_page, MEMORY_PAGE_SIZE);
			temp_phys_page(5, 0);
			temp_phys_page(4, 0);
		}
		temp_phys_page(3, 0);
		temp_phys_page(2, 0);
	}
	temp_phys_page(1, 0);
	temp_phys_page(0, 0);

	return new_phys_pd;
}

/**
* alloc_program_space - ladataan ohjelma omaan sivutauluunsa
* @phys_pagedir: sivuhakemiston fyysinen sijainti
* @size: koodin koko
*
* Palauttaa ensimmäisen varatun virtuaalisen sivun numeron
**/
uint_t alloc_program_space(uint_t phys_pagedir, uint_t size, const void *code, int user)
{
	if (!code || !size) {
		return -1;
	}
	return -1;

	uint_t page, pages = 1 + ((size - 1) / MEMORY_PAGE_SIZE);

	page = alloc_virtual_pages(phys_pagedir, pages, 1);

	return 0;
}

/**
* resize_stack - laajennetaan säikeen pinoa
* @phys_pagedir: sivuhakemiston fyysinen sijainti
* @stack_num: pinon numero (prosessin pinoista)
* @size: pinon uusi vähimmäiskoko (tavuina)
**/
int resize_stack(uint_t phys_pagedir, int stack_num, uint_t size, int user)
{
	if ((stack_num < 0 || stack_num > MAX_STACKS) || size > MAX_STACK_SIZE) {
		return -1;
	}
	uint_t pages, alloced, i;
	if (!size) {
		pages = 0;
	} else {
		pages = 1 + ((size - 1) / MEMORY_PAGE_SIZE);
	}

	i = STACK_TOP_PAGE(stack_num);
	alloced = 0;
	while (alloced < pages && temp_virt_page(0, phys_pagedir, i)) {
		temp_virt_page(0, 0, 0);
		++alloced;
		++i;
	}
	while (alloced < pages) {
		if (temp_virt_page(0, phys_pagedir, i)) {
			temp_virt_page(0, 0, 0);
		} else {
			if (map_virtual_page(phys_pagedir, i, 0, user) != 0) {
				panic("resize_stack: map_virtual_page != 0\n");
			}
		}
		++alloced;
		++i;
	}
	while (temp_virt_page(0, phys_pagedir, i)) {
		temp_virt_page(0, 0, 0);
		unmap_virtual_page(phys_pagedir, i);
		++i;
	}

	return 0;
}

/**
* free_pagedir - laajennetaan säikeen pinoa
* @phys_pagedir: sivuhakemiston fyysinen sijainti
* @size: koodin koko
**/
void free_pagedir(uint_t phys_pagedir)
{
	page_entry_t *pd;
	page_entry_t *pt;
	int i, j;

	if (phys_pagedir == KERNEL_PAGE_DIRECTORY) {
		panic("free_pagedir: freeing kernel!");
	}
	if (phys_pagedir == ADDR_TO_PAGE(asm_get_cr3())) {
		panic("free_pagedir: freeing active pages! O_o");
	}

	pd = temp_phys_page(0, phys_pagedir);
	for (i = 0; i < MEMORY_PE_COUNT; ++i) {
		if (!pd[i].pagenum) {
			continue;
		}
		pt = temp_phys_page(1, pd[i].pagenum);
		for (j = 0; j < MEMORY_PE_COUNT; ++j) {
			if (!pt[j].pagenum) {
				continue;
			}
			free_phys_page(pt[j].pagenum);
			pt[j] = NULL_PE;
		}
		temp_phys_page(1, 0);
		free_phys_page(pd[i].pagenum);
		pd[i] = NULL_PE;
	}
	temp_phys_page(0, 0);
	free_phys_page(phys_pagedir);
}

///==========================================================
///==========================================================
///==========================================================
uint_t physical_page_of(uint_t virt_page)
{
	return cur_virt_pt[virt_page].pagenum;
	//return (void *)(cur_virt_pt[((((uint_t)addr)>>22) * 1024) + ((((uint_t)addr)>>12) & 1023)] & 0xFFFFF000);
}

/* Search a free page and allocates given real page to given virtual address. Create new PDE if needed */
int _mmap(uint_t real_page, uint_t virtual_page, int user)
{
	uint_t pde_n = virtual_page >> 10;
	//uint_t pte_n = virtual_page & 0x3ff;
	uint_t free_page;

	// Do we have page table already?
	if (cur_virt_pd[pde_n].pagenum == 0) {
		/* Get memory */
		if (!(free_page = alloc_phys_page(PT_NOSWAP_FLAG))) {
			return -1;
		}

		// Set up new PDE
		cur_virt_pd[pde_n] = KERNEL_PE(free_page);

		// Write address of the new page table to the page table of memory window
		cur_virt_pt[KERNEL_PAGES + pde_n] = KERNEL_PE(free_page);
		asm_flush_cr3();
	}
	// TODO: mmap: jotain häikkää? O_o
	cur_virt_pt[virtual_page] = NEW_PE(real_page, user);
	asm_set_cr3(asm_get_cr3());
	return 0;
}

// Unmaps given virtual page
// TODO: Release the page table when all its entries are unmaped
void _unmap(uint_t virtual_page)
{
	uint_t pde_n = virtual_page >> 10;
	uint_t pte_n = virtual_page & 0x3ff;

	if (cur_virt_pd[pde_n].pagenum == 0) {
		panic("Illegal unmap (nonexistent page)!\n");
	}

	// TODO: unmap

	cur_virt_pt[pte_n] = NULL_PE;
	asm_set_cr3(asm_get_cr3());
}
