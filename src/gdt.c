#include <gdt.h>
#include <stdint.h>
#include <misc_asm.h>
#include <string.h>

struct gdt_entry gdt[5];
struct gdt_ptr gdt_ptr;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t privs, uint8_t exec)
{
	gdt[num].base_0_24 = base;
	gdt[num].base_24_32 = base >> 24;
	gdt[num].limit_0_16 = limit;
	gdt[num].limit_16_20 = limit >> 16;

	gdt[num].privs = privs;
	gdt[num].exec = exec;

	gdt[num].code_read_data_write = 1;

	gdt[num].present = 1;
	gdt[num].reserved_1 = 1;
	gdt[num].dir_conf = 0;
	gdt[num].accessed = 0;
	gdt[num].granularity = 1;
	gdt[num].is_32bit = 1;
	gdt[num].reserved_0 = 0;
}

void gdt_install(void)
{
	gdt_ptr.limit = sizeof(gdt);
	gdt_ptr.base = (unsigned int)&gdt;

	memset(gdt, 0, sizeof(gdt));

	// Kernel space:
	gdt_set_gate(1, 0, 0xFFFFF, 0, 1);
	gdt_set_gate(2, 0, 0xFFFFF, 0, 0);

	// User space
	gdt_set_gate(3, 0, 0xFFFFF, 3, 1);
	gdt_set_gate(4, 0, 0xFFFFF, 3, 0);

	asm_gdt_flush(&gdt_ptr);
}

