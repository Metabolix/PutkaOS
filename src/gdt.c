#include <gdt.h>

struct gdt_entry gdt[5];
struct gdt_ptr gdt_pointer;

extern void gdt_flush(void); /* This is defined in gdt.asm */

/* Setup a descriptor in GDT */
void gdt_set_gate(unsigned int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
	gdt[num].base_low = (base & 0xFFFF);    /* Setup the descriptor base address */
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	gdt[num].limit_low = (limit & 0xFFFF); /* Setup the descriptor limits */

	gdt[num].granularity = ((limit >> 16) & 0x0F);


	gdt[num].granularity |= (gran & 0xF0); /* Setup the granularity and access flags */
	gdt[num].access = access;
}

void gdt_install(void)
{
	gdt_pointer.limit = (sizeof(struct gdt_entry) * 5) - 1;	/* Limit */
	gdt_pointer.base = (unsigned int)&gdt;		/* Base address */

	gdt_set_gate(0, 0, 0, 0, 0); /* NULL descriptor */

	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Code Segment */

	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Data Segment */

	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* Userspace data */
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* Userspace code */

	gdt_flush();
}

