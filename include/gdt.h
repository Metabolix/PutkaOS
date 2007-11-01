#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>

struct gdt_entry
{
	unsigned int
		limit_0_16 : 16,
		base_0_24 : 24,

		accessed : 1,
		code_read_data_write : 1,
		dir_conf : 1,
		exec : 1,
		reserved_1 : 1,
		privs : 2,
		present : 1,

		limit_16_20 : 4,
		reserved_0 : 2,
		is_32bit : 1,
		granularity : 1,

		base_24_32 : 8;
} __attribute__((packed));
/*
struct gdt_entry
{
	unsigned int
		limit_0_16 : 16,
		base_0_24 : 24,

		present : 1,
		privs : 2,
		reserved_1 : 1,
		exec : 1,
		dir_conf : 1,
		code_read_data_write : 1,

		accessed : 1,
		granularity : 1,
		is_32bit : 1,
		reserved_0 : 2,
		limit_16_20 : 4,
		base_24_32 : 8;
} __attribute__((packed));

struct gdt_entry
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));
*/
struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

//void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
extern void gdt_install(void);

#endif
