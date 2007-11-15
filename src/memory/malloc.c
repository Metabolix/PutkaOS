#include <memory/memory.h>
#include <memory/malloc.h>
#include <memory/kmalloc.h>
#include <string.h>
#include <panic.h>
#include <screen.h>

#define MAX_ALLOCATIONS 512

typedef struct
{
	size_t size;
	pid_t pid;
	int user;
	void * virt_addr;
} area;

// Function prototypes
static void * sysmalloc(size_t size, int user);
static int sysfree(void * ptr, int user);
static void * sysrealloc(void *ptr, size_t size, int user);

// An ugly way to save allocation sizes...
area memory_allocations[MAX_ALLOCATIONS] = {{0}};

void malloc_init(void)
{
	memset(memory_allocations, 0, sizeof(area) * MAX_ALLOCATIONS);
}

static void * sysmalloc(size_t size, int user)
{
	if (memory_free() < size) {
		return 0; /* Out of physical memory */
	}
	if (!size) {
		return 0;
	}
	// TODO: sysmalloc: varataan jo varatuilta sivuilta vapaat osat
	size = (1 + (size-1)/MEMORY_PAGE_SIZE) * MEMORY_PAGE_SIZE;

	uint_t pages = size / MEMORY_PAGE_SIZE;
	uint_t virt_page;
	int i;

	// Etsitään vapaa paikka malloc-taulusta
	i = 0;
	while (memory_allocations[i].size != 0) {
		++i;
		if (i == MAX_ALLOCATIONS) {
			print("i == MAX_ALLOCATIONS\n");
			return 0; /* Woops... allocation table is full :/ */
		}
	}

	// Varataan tila
	if (!(virt_page = alloc_virtual_pages(0, pages, user))) {
		print("virt_page = 0\n");
		return 0; /* Out of linear memory */
	}

	memory_allocations[i].virt_addr = (void*)(virt_page * MEMORY_PAGE_SIZE);
	memory_allocations[i].size = size;
	memory_allocations[i].pid = active_pid;
	memory_allocations[i].user = user;

	if (active_pid == NO_PROCESS) {
		//print("Varattiin vaarallista muistia!\n");
	}

	return memory_allocations[i].virt_addr;
}

static int sysfree(void * ptr, int user)
{
	uint_t address;
	uint_t virt_page_beg, offset_beg;
	uint_t virt_page_end, offset_end;
	int i;

	if (user && (ADDR_TO_PAGE(ptr) < USER_PAGES_BEG)) {
		return -1; /* Someone tries to do something nasty by releasing kernel memory */
	}

	for (i = 0; i < MAX_ALLOCATIONS; ++i) {
		if (memory_allocations[i].virt_addr == ptr) {
			if (memory_allocations[i].pid == active_pid) {
				break;
			}
			if (!memory_allocations[i].user && !user && memory_allocations[i].pid == NO_PROCESS) {
				print("Vapautettiin vaarallista muistia <3\n");
				break;
			}
		}
	}
	if (i == MAX_ALLOCATIONS) {
		print("Ei vapautettu muistia. :(\n");
		if (!user) {
			panic("kfree: viallinen vapautettava!\n");
		}
		return -1;
	}

	address = (uint_t) memory_allocations[i].virt_addr;
	offset_beg = address & 0xfff;
	virt_page_beg = address >> 12;

	address += memory_allocations[i].size;
	offset_end = address & 0xfff;
	virt_page_end = (address >> 12);

	memset(&memory_allocations[i], 0, sizeof(memory_allocations[i]));

	if (offset_beg) {
		panic("sysfree: offset_beg != 0\n");
		// TODO: sysfree: "vapautetaan" osittaiset sivut
		// Vapautetaan virt_page_beg
		++virt_page_beg;
	}
	if (offset_end) {
		panic("sysfree: offset_end != 0\n");
		// Vapautetaan virt_page_end
	}
	while (virt_page_beg < virt_page_end) {
		unmap_virtual_page(0, virt_page_beg);
		++virt_page_beg;
	}
	return 0;
}

static void * sysrealloc(void *ptr, size_t size, int user)
{
	if (!ptr) {
		return sysmalloc(size, user);
	}
	if (!size) {
		sysfree(ptr, user);
		return 0;
	}

	// TODO: sysrealloc: voiko jatkaa samasta kohti?
	void * retval = sysmalloc(size, user);
	if (!retval) {
		return 0;
	}

	// TODO: sysrealloc: old block size? memcpy(new, old, MIN(new_size, old_size));
	memcpy(retval, ptr, size);
	sysfree(ptr, user);
	return retval;
}

/**
* Kernel functions
**/

void * kmalloc(size_t size) {
	return sysmalloc(size, 0);
}

void kfree(void * ptr) {
	sysfree(ptr, 0);
}

void * krealloc(void *ptr, size_t size)
{
	return sysrealloc(ptr, size, 0);
}

void * kcalloc(size_t nmemb, size_t size)
{
	void *retval = kmalloc(nmemb * size);
	if (retval) {
		memset(retval, 0, nmemb * size);
	}
	return retval;
}

/**
* User functions:
*****/
/**
* syscall_malloc: malloc(ecx);
**/
void * syscall_malloc(size_t size)
{
	return sysmalloc(size, 1);
}

/**
* syscall_free: free(ecx);
**/
void syscall_free(void *ptr)
{
	sysfree(ptr, 1);
}

/**
* syscall_realloc: realloc(ecx, edx);
**/
void * syscall_realloc(void *ptr, size_t size)
{
	return sysrealloc(ptr, size, 1);
}
