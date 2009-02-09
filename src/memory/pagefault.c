#include <memory/memory.h>
#include <memory/pagefault.h>
#include <multitasking/scheduler.h>
#include <multitasking/multitasking.h>
#include <multitasking/process.h>
#include <multitasking/thread.h>
#include <misc_asm.h>
#include <stdio.h>
#include <tss.h>
#include <panic.h>
#include <string.h>
#include <kprintf.h>

extern struct kernel_tasks kernel_tasks;

static uint_t phys_pagedir;

static char * cr2;
static uint_t dpage, doffset;
static uint_t dpde, dpte;

static char * eip;
static uint_t cpage, coffset;
static uint_t cpde, cpte;

static int user_tries_kernel(void)
{
	int r = 0;
	char *code, *code2;
	code = temp_virt_page(0, phys_pagedir, cpage);
	code2 = temp_virt_page(1, phys_pagedir, cpage + 1);

	if (memcmp(code + coffset, "\x65\xa1\x14\x00\x00\x00", 6) == 0) {
		kprintf("Ohjelmassa on Linuxin stack protector!\nAnna gcc:llesi lippu -fno-stack-protector\n");
		r = -1;
		goto return_etc;
	}

	kprintf("Page Fault!\nThread %i, process %i\n", active_tid, active_pid);
	kprintf("Trying to access address %p (page %d).\n", cr2, dpage);
	kprintf("Page fault; page-level protection violation (no right)!\n");
	r = -1;
	goto return_etc;

return_etc:
	temp_virt_page(0, 0, 0);
	if (code2) temp_virt_page(1, 0, 0);
	return r;
}

int handle_user_pagefault(void)
{
	phys_pagedir = active_process->mem.phys_pd;

	cr2 = asm_get_cr2();
	dpage = ADDR_TO_PAGE(cr2);
	doffset = eip - (char*) PAGE_TO_ADDR(dpage);
	dpde = dpage / MEMORY_PE_COUNT;
	dpte = dpage % MEMORY_PE_COUNT;

	eip = (void*) kernel_tasks.tss_for_active_thread.eip;
	cpage = ADDR_TO_PAGE(eip);
	coffset = eip - (char*) PAGE_TO_ADDR(cpage);
	cpde = cpage / MEMORY_PE_COUNT;
	cpte = cpage % MEMORY_PE_COUNT;

	const char *panic_msg;
	uint_t new_location;

	page_entry_t *pd, *pt;

	if (!(pd = temp_page_directory(phys_pagedir))) {
		goto no_pd_got;
	}
	if (!pd[dpde].pagenum) {
		goto no_pt_page;
	}
	if ((dpde >= KMEM_PDE_END) && !pd[dpde].user) {
		printf("User process = %d, thread = %d\n", active_pid, active_tid);
		printf("Trying to access address %p (page %d).\n", asm_get_cr2(), ADDR_TO_PAGE(asm_get_cr2()));
		printf("(!pd[dpde].user)\n");
		panic("Bug in memory handling!");
	}
	if (!pd[dpde].present) {
		new_location = swap_in(phys_pagedir, pd[dpde].pagenum);
		if (!new_location) {
			goto no_pt_page_swapped;
		}
		pd[dpde].pagenum = new_location;
		pd[dpde].present = 1;
	}
	if (!(pt = temp_page_table(pd[dpde].pagenum))) {
		goto no_pt_got;
	}
	if (dpde && dpte && !pt[dpte].pagenum) {
		goto no_cr2_page;
	}
	if (!pt[dpte].user) {
		return user_tries_kernel();
	}
	if (dpde < KMEM_PDE_END) {
		printf("User process = %d, thread = %d\n", active_pid, active_tid);
		printf("Trying to access address %p (page %d).\n", asm_get_cr2(), ADDR_TO_PAGE(asm_get_cr2()));
		printf("(dpde < KMEM_PDE_END) && pt[dpte].user)\n");
		panic("Bug in memory handling!");
		return user_tries_kernel();
	}
	if (!pt[dpte].present) {
		new_location = swap_in(phys_pagedir, pt[dpte].pagenum);
		if (!new_location) {
			goto no_cr2_page_swapped;
		}
		pt[dpte].pagenum = new_location;
		pt[dpte].present = 1;
	}

	return 0; // Ratkaistu. :)

no_pd_got:
	panic_msg = "Page fault: failed getting PD from RAM!";
	goto fail;
no_pt_page:
	panic_msg = "Page fault; page missing from PD!";
	goto fail;
no_pt_page_swapped:
	panic_msg = "Page fault; failed swapping PT to RAM!";
	goto fail;
no_pt_got:
	panic_msg = "Page fault; failed getting PT from RAM!";
	goto fail;
no_cr2_page:
	panic_msg = "Page fault; page missing from PT!";
	goto fail;
no_cr2_page_swapped:
	panic_msg = "Page fault; failed swapping page to RAM!";
	goto fail;

fail:
	printf("Page Fault!\nThread %i, process %i\n", active_tid, active_pid);
	printf("Trying to access address %p (page %d).\n", cr2, dpage);
	printf("%s\n", panic_msg);
	return -1;
}

void page_fault_handler(void)
{
	if (!has_threading() || (kernel_tasks.tss_for_active_thread.cs & 3) == 0) {
		kprintf("Kernel process = %d, thread = %d\n", active_pid, active_tid);
		kprintf("Trying to access address %p (page %d).\n", asm_get_cr2(), ADDR_TO_PAGE(asm_get_cr2()));
		panic("Page fault in kernel!");
		return;
	}
	int i = handle_user_pagefault();
	if (i) {
		tid_t tid = active_tid;
		while (tid == active_tid) {
			scheduler();
		}
		kill_thread(tid);
	}
	return;
}
