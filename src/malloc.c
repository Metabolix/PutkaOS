#include <memory.h>
#include <string.h>
#include <panic.h>
#include <screen.h>

typedef struct
{
	void * virtual_start;
	size_t size;
	void * physical_start;
} area;

// Function prototypes
static void * sysmalloc(size_t size, unsigned char type);
static void sysfree(void * pointer, unsigned char type);
static void * sysrealloc(void *ptr, size_t size, unsigned char type);

// An ugly way to save allocation sizes...
area memory_allocations[MAX_ALLOCATIONS];

void malloc_init(void)
{
	memset(memory_allocations, 0, sizeof(area) * MAX_ALLOCATIONS);
}

static void * sysmalloc(size_t size, unsigned char type)
{
	if(memory_free() < size)
		return 0; /* Out of physical memory */

	void * area_start = find_area(size, type);
	unsigned int block, address, cnt0;

	if(area_start==0)
		return 0; /* Out of linear memory */

	for(cnt0=0; memory_allocations[cnt0].size!=0x0 && cnt0< MAX_ALLOCATIONS; cnt0++) ;

	if(cnt0>=MAX_ALLOCATIONS)
		return 0; /* Woops... allocation table is full :/ */

	block = find_free_block();
	memory_allocations[cnt0].virtual_start =  area_start;
	memory_allocations[cnt0].size = size;
	memory_allocations[cnt0].physical_start = (void *) (block * MEMORY_BLOCK_SIZE); /* Used to identify every allocation table entry */

	for(address=((unsigned int) area_start); address<(((unsigned int) (area_start)) + size); address+=MEMORY_BLOCK_SIZE)
	{
		reserve_block(block);
		if(mmap((void *)(block * MEMORY_BLOCK_SIZE), (void *)address, type))
			panic("Out of memory... couldn't create a page table.");

		block = find_free_block();
	}
	return area_start;
}

void * malloc(size_t size) {
	return sysmalloc(size, USER_PAGE_PARAMS);
}

void * kmalloc(size_t size) {
	return sysmalloc(size, KERNEL_PAGE_PARAMS);
}


static void sysfree(void * pointer, unsigned char type)
{
	unsigned int cnt0, block, address;
	void * physical_addr = get_physical_address(pointer);

	if(((unsigned int)pointer)<KERNEL_PDE_COUNT*1024*MEMORY_BLOCK_SIZE && type==USER_PAGE_PARAMS)
		return; /* Someone tries to do something nasty by releasing kernel memory */

	for(cnt0=0; memory_allocations[cnt0].physical_start!=physical_addr && cnt0< MAX_ALLOCATIONS; cnt0++) ;

	if(physical_addr==memory_allocations[cnt0].physical_start)
	{
		for(address=((unsigned int)memory_allocations[cnt0].virtual_start); address<(((unsigned int)memory_allocations[cnt0].virtual_start)+memory_allocations[cnt0].size); address+=MEMORY_BLOCK_SIZE)
		{
			block = ((unsigned int) get_physical_address((void *)address)) / MEMORY_BLOCK_SIZE;
			release_block(block);
			unmap((void *)address);
		}
	}
	memory_allocations[cnt0].size=0x0;
	memory_allocations[cnt0].virtual_start=0x0;
	memory_allocations[cnt0].physical_start=0x0;
}

void kfree(void * pointer) {
	sysfree(pointer, KERNEL_PAGE_PARAMS);
}

void free(void * pointer) {
	sysfree(pointer, USER_PAGE_PARAMS);
}

static void * sysrealloc(void *ptr, size_t size, unsigned char type)
{
	if (!ptr) {
		return sysmalloc(size,type);
	}
	if (!size) {
		sysfree(ptr,type);
		return 0;
	}

	void * retval = sysmalloc(size,type);
	if(retval==0)
		return 0;

	memcpy(retval, ptr, size);
	sysfree(ptr,type);
	return retval;
}

void * realloc(void *ptr, size_t size)
{
	return sysrealloc(ptr, size, USER_PAGE_PARAMS);
}

void * krealloc(void *ptr, size_t size)
{
	return sysrealloc(ptr, size, KERNEL_PAGE_PARAMS);
}

void * kcalloc(size_t nmemb, size_t size)
{
	void *retval = kmalloc(nmemb * size);
	if (retval)
		memset(retval, 0, nmemb * size);
	return retval;
}

void * calloc(size_t nmemb, size_t size)
{
	void *retval = malloc(nmemb * size);
	if (retval)
		memset(retval, 0, nmemb * size);
	return retval;
}
