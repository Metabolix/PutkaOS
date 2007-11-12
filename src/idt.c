#include <idt.h>
#include <gdt.h>
#include <string.h>
#include <misc_asm.h>
#include <stdint.h>

struct idt_entry idt[256] = {{0}};
struct idt_ptr idt_pointer;

void idt_set_interrupt(uint8_t num, void (*base)(), uint8_t priv)
{
	memset(&idt[num], 0, sizeof(idt[num]));
	idt[num].base_00_16 = (uintptr_t) base;
	idt[num].base_16_32 = (uintptr_t) base >> 16;
	idt[num].type = IDT_32b_INTERRUPT;
	idt[num].priv = priv;

	idt[num].sel = KERNEL_CS_SEL;
	idt[num].present = 1;
}

void idt_set_task_gate(uint8_t num, uint16_t sel, uint8_t priv)
{
	memset(&idt[num], 0, sizeof(idt[num]));
	idt[num].sel = sel;
	idt[num].type = IDT_32b_TASK_GATE;
	idt[num].present = 1;
	idt[num].priv = priv;
}

void idt_install(void)
{
	idt_pointer.limit = sizeof(idt) - 1;
	idt_pointer.base = idt;

	asm_idt_load(&idt_pointer);
}

