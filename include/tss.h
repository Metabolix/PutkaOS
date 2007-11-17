#ifndef _TSS_H
#define _TSS_H 1

#include <stdint.h>

struct tss {
	uint16_t link, reserved00;
	uint32_t esp0, ss0;
	uint32_t esp1, ss1;
	uint32_t esp2, ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax, ecx, edx, ebx;
	uint32_t esp, ebp, esi, edi;
	uint32_t es, cs, ss, ds, fs, gs;
	uint32_t ldtr;
	uint16_t reserved01, iopb_offset;
};

struct kernel_tasks {
	struct tss tss_for_hw_int;
	struct tss tss_for_page_fault;
	struct tss tss_for_double_fault;
	struct tss tss_for_active_thread;
};

#endif
