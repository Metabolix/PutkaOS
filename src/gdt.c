#include <gdt.h>
#include <stdint.h>
#include <misc_asm.h>
#include <string.h>
#include <memory/memory.h>
#include <memory/pagefault.h>
#include <doublefault.h>
#include <multitasking/multitasking.h>
#include <multitasking/process.h>
#include <multitasking/thread.h>
#include <idt.h>
#include <tss.h>

extern char stack_for_page_fault;
extern char stack_for_double_fault;
extern char stack_for_hw_int_task;
extern char stack_for_hw_interrupt;
extern void irq_task(void);

union gdt_entry gdt[32];

struct gdt_ptr gdt_ptr;

struct kernel_tasks kernel_tasks = {
	.tss_for_hw_int = {
		.cs = KERNEL_CS_SEL,
		.eip = (uintptr_t) irq_task,
		.ds = KERNEL_DS_SEL,
		.es = KERNEL_DS_SEL,
		.fs = KERNEL_DS_SEL,
		.gs = KERNEL_DS_SEL,
		.ss = KERNEL_DS_SEL,
		.esp = (uintptr_t) &stack_for_hw_int_task,
		.ebp = (uintptr_t) &stack_for_hw_int_task,
		.cr3 = (uintptr_t) PAGE_TO_ADDR(KERNEL_PAGE_DIRECTORY),
		.eflags = 0x02,
	},
	.tss_for_page_fault = {
		.cs = KERNEL_CS_SEL,
		.eip = (uintptr_t) asm_page_fault_handler,
		.ds = KERNEL_DS_SEL,
		.es = KERNEL_DS_SEL,
		.fs = KERNEL_DS_SEL,
		.gs = KERNEL_DS_SEL,
		.ss = KERNEL_DS_SEL,
		.esp = (uintptr_t) &stack_for_page_fault,
		.ebp = (uintptr_t) &stack_for_page_fault,
		.cr3 = (uintptr_t) PAGE_TO_ADDR(KERNEL_PAGE_DIRECTORY),
		.eflags = 0x02,
	},
	.tss_for_double_fault = {
		.cs = KERNEL_CS_SEL,
		.eip = (uintptr_t) asm_double_fault_handler,
		.ds = KERNEL_DS_SEL,
		.es = KERNEL_DS_SEL,
		.fs = KERNEL_DS_SEL,
		.gs = KERNEL_DS_SEL,
		.ss = KERNEL_DS_SEL,
		.esp = (uintptr_t) &stack_for_double_fault,
		.ebp = (uintptr_t) &stack_for_double_fault,
		.cr3 = (uintptr_t) PAGE_TO_ADDR(KERNEL_PAGE_DIRECTORY),
		.eflags = 0x02,
	},
	.tss_for_active_thread = {
		.ss0 = KERNEL_DS_SEL,
		.esp0 = (uintptr_t) &stack_for_hw_interrupt,
		.cr3 = (uintptr_t) PAGE_TO_ADDR(KERNEL_PAGE_DIRECTORY),
		.eflags = 0x02,
	},
};

void gdt_set_gate(struct gdt_memseg_desc *ptr, uint32_t base, uint32_t limit, uint8_t privs, uint8_t exec)
{
	ptr->base_0_24 = base;
	ptr->base_24_32 = base >> 24;
	ptr->limit_0_16 = limit;
	ptr->limit_16_20 = limit >> 16;

	ptr->privs = privs;
	ptr->exec = exec;

	ptr->code_read_data_write = 1;

	ptr->present = 1;
	ptr->segment = 1;
	ptr->dir_conf = 0;
	ptr->accessed = 0;
	ptr->granularity = 1;
	ptr->is_32bit = 1;
	ptr->is_ia32_64bit = 0;
	ptr->system_segment = 0;
}

void gdt_set_tss(struct gdt_tss_desc *gdt_entry, struct tss *tss_ptr)
{
	gdt_entry->base_0_24 = (uintptr_t) tss_ptr;
	gdt_entry->base_24_32 = (uintptr_t) tss_ptr >> 24;
	gdt_entry->limit_0_16 = sizeof(struct tss) - 1;
	gdt_entry->limit_16_20 = (sizeof(struct tss) - 1) >> 16;

	gdt_entry->accessed = 1;
	gdt_entry->code_read_data_write = 0;
	gdt_entry->dir_conf = 0;
	gdt_entry->exec = 1;
	gdt_entry->segment = 0;
	gdt_entry->privs = 0;
	gdt_entry->present = 1;

	gdt_entry->granularity = 0;
	gdt_entry->is_32bit = 1;
	gdt_entry->is_ia32_64bit = 0;
	gdt_entry->system_segment = 1;

	tss_ptr->iopb_offset = sizeof(struct tss);
	//tss[num].link = num ? 0x28 : 0x30;
}

extern void asm_set_tr(uint16_t sel);
extern uint16_t asm_get_tr();

void gdt_install(void)
{
	gdt_ptr.limit = sizeof(gdt) - 1;
	gdt_ptr.base = (unsigned int)&gdt;

	memset(gdt, 0, sizeof(gdt));

	// Kernel space
	gdt_set_gate(&gdt[GDT_KERNEL_CS].memseg, 0, 0xFFFFF, 0, 1);
	gdt_set_gate(&gdt[GDT_KERNEL_DS].memseg, 0, 0xFFFFF, 0, 0);

	// User space
	gdt_set_gate(&gdt[GDT_USER_CS].memseg, 0, 0xFFFFF, 3, 1);
	gdt_set_gate(&gdt[GDT_USER_DS].memseg, 0, 0xFFFFF, 3, 0);

	// IRQ, faults, thread
	gdt_set_tss(&gdt[GDT_HW_INT_TSS].tss, &kernel_tasks.tss_for_hw_int);
	gdt_set_tss(&gdt[GDT_PAGE_FAULT_TSS].tss, &kernel_tasks.tss_for_page_fault);
	gdt_set_tss(&gdt[GDT_DOUBLE_FAULT_TSS].tss, &kernel_tasks.tss_for_double_fault);
	gdt_set_tss(&gdt[GDT_ACTIVE_THREAD_TSS].tss, &kernel_tasks.tss_for_active_thread);

	asm_gdt_flush(&gdt_ptr);
	asm_set_tr(8 * GDT_ACTIVE_THREAD_TSS);
}
