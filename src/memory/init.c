#include <memory/memory.h>
#include <memory/memvars.h>
#include <stdint.h>
#include <misc_asm.h>
#include <screen.h>
#include <panic.h>

uint32_t * const phys_pages_alloced = (uint32_t *) (PHYSICAL_PAGES_ALLOCATION_BITMAP * MEMORY_PAGE_SIZE);
uint32_t * const phys_pages_noswap = (uint32_t *) (PHYSICAL_PAGES_NOSWAP_BITMAP * MEMORY_PAGE_SIZE);

uint_t first_phys_pages_not_alloced = 0;

struct memory_info memory = {0};

/**
* init_phys_page_bitmaps - alustetaan bittikartta fyysisistä sivuista
* @mem_kib: muistin kokonaismäärä (KiB)
**/
void init_phys_page_bitmaps(uint_t mem_kib)
{
	memory.ram_pages = mem_kib / (MEMORY_PAGE_SIZE / 1024);
	memory.kernel_ram_pages = KERNEL_PAGES;
	memory.ram_pages_free = 0;

	const uint_t k = KERNEL_PAGES;
	const uint_t p = mem_kib / (MEMORY_PAGE_SIZE / 1024);

	uint_t zero_start;
	uint_t zero_end = p / 32;

	memset(phys_pages_alloced, 0xff, PHYS_PAGE_BITMAP_LEN32 * 4);
	memset(phys_pages_noswap, 0xff, PHYS_PAGE_BITMAP_LEN32 * 4);
	if (k % 32) {
		memory.ram_pages_free += (32 - (k % 32));
		phys_pages_alloced[k / 32] = 0xffffffff >> (32 - (k % 32));
		phys_pages_alloced[k / 32] = 0xffffffff >> (32 - (k % 32));
		zero_start = (k / 32) + 1;
	} else {
		zero_start = k / 32;
	}

	if (zero_end <= zero_start) {
		kprintf("Kernelille %d, saatavilla %d (= %d); %d <= %d.\n", k, p, mem_kib, zero_end, zero_start);
		panic("Osta isompi muisti!\n");
	}

	memory.ram_pages_free += 32 * (zero_end - zero_start);
	memset(phys_pages_alloced + zero_start, 0, (zero_end - zero_start) * 4);
	memset(phys_pages_noswap + zero_start, 0, (zero_end - zero_start) * 4);
	if (p % 32) {
		memory.ram_pages_free += (p % 32);
		phys_pages_alloced[p / 32] = 0xffffffff << (p % 32);
		phys_pages_alloced[p / 32] = 0xffffffff << (p % 32);
	}
}

/**
* memory_check_ram_size - selvitetään RAMin määrä (KiB)
**/
uint_t memory_check_ram_size(uint_t arvio)
{
	char *ptr = 0, c, d;
	uint_t min = (((uint_t)(memory_check_ram_size)) + 1023) / 1024;
	uint_t max = 4 * 1024 * 1024;
	uint_t x = arvio - 1;
	if (x < min) { x = min; }
	if (x > max) { x = max; }
	do {
		c = ptr[1024 * x];
		d = c + 0x55;

		ptr[1024 * x] = d;
		asm_nop(ptr);
		if (ptr[1024 * x] != d) {
			max = x;
			continue;
		}

		ptr[1024 * x] = c;
		asm_nop(ptr);
		if (ptr[1024 * x] != c) {
			max = x;
			continue;
		}
		min = x;
	} while ((x = (min + max) / 2) != min);
	return min + 1;
}

/**
* memory_init - alustetaan muistinhallinta
* @mem_kib: muistin kokonaismäärä (KiB)
**/
void memory_init(uint_t mem_kib)
{
	uint_t i;

	// Selvitetään muistin oikea määrä itse
	mem_kib = memory_check_ram_size(mem_kib);

	// Merkitään, missä loppuu kernelin muisti ja missä koko muisti
	init_phys_page_bitmaps(mem_kib);

	// Nollataan taulut
	memset(kernel_page_directory, 0, MEMORY_PAGE_SIZE);
	memset(kernel_page_tables, 0, MEMORY_PAGE_SIZE * KERNEL_PDE_COUNT);

	// Page directory osoittaa tietenkin sivutauluihin
	for (i = 0; i < KERNEL_PDE_COUNT; i++) {
		kernel_page_directory[i] = KERNEL_PE(KERNEL_PAGE_TABLES + i);
	}

	// Suora fyysinen muisti (KERNEL_PAGES)
	for (i = 0; i < KERNEL_PAGES; i++) {
		kernel_page_tables[i] = KERNEL_PE(i);
	}

	// Viimeinen "kernelin sivu" osoittaa directoryyn
	kernel_page_tables[KERNEL_PAGES - 1] = KERNEL_PE(KERNEL_PAGE_DIRECTORY);

	// Muisti kernelin sivujen jälkeen osoittaa sivutauluihin
	for (i = 0; i < KERNEL_PDE_COUNT; i++) {
		kernel_page_tables[KERNEL_PAGES + i] = KERNEL_PE(KERNEL_PAGE_TABLES + i);
	}

	// Sitten kerrotaan asioista prosessorille
	asm_set_cr3(kernel_page_directory);
	asm_set_cr0(asm_get_cr0() | 0x80000000);

	if (!(asm_get_cr0() & 0x80000000)) {
		panic("Muistin sivutus ei toimi!\n");
	}

	const uint_t kib1 = memory.ram_pages * MEMORY_PAGE_SIZE / 1024;
	const uint_t kib2 = memory.ram_pages_free * MEMORY_PAGE_SIZE / 1024;
	kprintf("Memory management is enabled. RAM: %d.%02d MiB (free %d.%02d MiB)\n",
		kib1/1024, (100 * (kib1 % 1024)) / 1024,
		kib2/1024, (100 * (kib2 % 1024)) / 1024);
	//kprintf("MEM: Using %u kilobytes of %u available\n", memory, ram_count);
}
