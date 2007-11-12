#ifndef _GDT_H
#define _GDT_H 1

#include <stdint.h>

/**
* GDT setup:
* 0x00 : NULL
* 0x01 : HW IRQ TSS
* 0x02 : Page fault TSS
* 0x03 : Double fault TSS
* 0x04 : Active task TSS
* 0x10 : Kernel CS
* 0x11 : Kernel DS
* 0x12 : User CS
* 0x13 : User DS
**/

#define GDT_KERNEL_CS (0x10)
#define GDT_KERNEL_DS (0x11)
#define GDT_USER_CS (0x12)
#define GDT_USER_DS (0x13)

#define GDT_HW_INT_TSS (0x01)
#define GDT_PAGE_FAULT_TSS (0x02)
#define GDT_DOUBLE_FAULT_TSS (0x03)
#define GDT_ACTIVE_THREAD_TSS (0x04)

#define KERNEL_CS_SEL (8 * GDT_KERNEL_CS)
#define KERNEL_DS_SEL (8 * GDT_KERNEL_DS)
#define USER_CS_SEL ((8 * GDT_USER_CS) | 3)
#define USER_DS_SEL ((8 * GDT_USER_DS) | 3)

struct gdt_memseg_desc
{
	unsigned int
		limit_0_16 : 16,
		base_0_24 : 24,

		accessed : 1,
		code_read_data_write : 1,
		dir_conf : 1,
		exec : 1,
		segment : 1,
		privs : 2,
		present : 1,

		limit_16_20 : 4,
		system_segment : 1,
		is_ia32_64bit : 1,
		is_32bit : 1,
		granularity : 1,

		base_24_32 : 8;
} __attribute__((packed));

struct gdt_tss_desc
{
	unsigned int
		limit_0_16 : 16,
		base_0_24 : 24,

		accessed : 1,
		code_read_data_write : 1,
		dir_conf : 1,
		exec : 1,
		segment : 1,
		privs : 2,
		present : 1,

		limit_16_20 : 4,
		system_segment : 1,
		is_ia32_64bit : 1,
		is_32bit : 1,
		granularity : 1,

		base_24_32 : 8;
} __attribute__((packed));

union gdt_entry {
	struct gdt_memseg_desc memseg;
	struct gdt_tss_desc tss;
} __attribute__((packed));

struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

//void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
extern void gdt_install(void);

#endif
