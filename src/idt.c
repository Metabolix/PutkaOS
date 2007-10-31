#include <idt.h>
#include <string.h>
#include <misc_asm.h>
#include <stdint.h>

struct idt_entry idt[256]; /* table of idt_entries */
struct idt_ptr idt_pointer;

void idt_set_gate(uint8_t num, uint_t base, uint16_t sel, uint8_t flags)
{
	idt[num].base_lo = (base & 0xFFFF);    /* The interrupt routine's base address */
	idt[num].base_hi = (base >> 16) & 0xFFFF;

	idt[num].sel = sel; /* Set some flags */
	idt[num].always0 = 0;
	idt[num].flags = flags;
}

void idt_install(void)
{
	idt_pointer.limit = (sizeof (struct idt_entry) * 256) - 1; /* 256 entries */
	idt_pointer.base = (unsigned int)&idt;

	memset(&idt, 0, sizeof(struct idt_entry) * 256); /* init them */

	asm_idt_load(&idt_pointer); /* points processor register to our IDT */
}

