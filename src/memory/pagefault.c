#include <memory/memory.h>
#include <memory/pagefault.h>
#include <multitasking/multitasking.h>
#include <multitasking/process.h>
#include <multitasking/thread.h>
#include <misc_asm.h>
#include <screen.h>
#include <panic.h>

void page_fault_handler(void)
{
	panic("Page fault! (TODO)");
#if 0
	void * const cr2_ptr = asm_get_cr2();
	const uint_t page = ((uintptr_t) cr2_ptr) >> 12;
	const uint_t phys_pagedir = active_process->mem.phys_pd;
	const uint_t pde = page / MEMORY_PE_COUNT, pte = page % MEMORY_PE_COUNT;
	const char *panic_msg;
	uint_t new_location;

	page_entry_t *pd, *pt;

	if (!(regs->error_code & 1)) {
		goto no_right;
	}

	if (!active_process) {
		goto no_process;
	}

	if (!(pd = temp_page_directory(phys_pagedir))) {
		goto no_pd_got;
	}
	if (!pd[pde].pagenum) {
		goto no_pt_page;
	}
	if (!pd[pde].present) {
		new_location = swap_in(phys_pagedir, pd[pde].pagenum);
		if (!new_location) {
			goto no_pt_page_swapped;
		}
		pd[pde].pagenum = new_location;
		pd[pde].present = 1;
	}
	if (!(pt = temp_page_table(pd[pde].pagenum))) {
		goto no_pt_got;
	}
	if (!pt[pte].pagenum) {
		goto no_cr2_page;
	}
	if (!pt[pte].present) {
		new_location = swap_in(phys_pagedir, pt[pte].pagenum);
		if (!new_location) {
			goto no_cr2_page_swapped;
		}
		pt[pte].pagenum = new_location;
		pt[pte].present = 1;
	}
	return;
no_right:
	panic_msg = "Page fault; page-level protection violation (no right / no page)!";
	goto fail;
no_process:
	panic_msg = "Page fault && (active_process == 0)!";
	goto fail;
no_pd_got:
	panic_msg = "Page fault && failed getting page directory from RAM!";
	goto fail;
no_pt_page:
	panic_msg = "Page fault && page directory doesn't contain this PDE!";
	goto fail;
no_pt_page_swapped:
	panic_msg = "Page fault && failed swapping page table to RAM!";
	goto fail;
no_pt_got:
	panic_msg = "Page fault && failed getting page table from RAM!";
	goto fail;
no_cr2_page:
	panic_msg = "Page fault && page table doesn't contain this PTE!";
	goto fail;
no_cr2_page_swapped:
	panic_msg = "Page fault && failed swapping page to RAM!";
	goto fail;

fail:
	kprintf("Page Fault!\nThread %i, process %i\n", active_tid, active_pid);
	kprintf("Trying to %s address %p (page %d).\n", ((regs->error_code & 2) ? "write" : "read"), cr2_ptr, page);
	kprintf("Processor is in %s mode.\n", ((regs->error_code & 4) ? "user" : "supervisor"));
	print(panic_msg);
	print("\n");
	dump_regs(regs);
	*/
	panic(panic_msg);
#endif
}
