#ifndef _IDT_H
#define _IDT_H 1

#include <stdint.h>

/**
* Interrupt mappings:
* 0x00 - 0x1f : CPU faults
* 0x20 - 0x2f : HW interrupts
* 0x40        : Thread ending
* 0x41        : Scheduler (switch thread)
* 0x80        : Syscall
**/

#define IDT_THREAD_ENDING (0x40)
#define IDT_SCHEDULER (0x41)
#define IDT_SYSCALL (0x80)

#define IDT_32b_INTERRUPT (0xE)
#define IDT_32b_TASK_GATE (0x5)

#define IDT_PRIV_KERNEL (0)
#define IDT_PRIV_USER (3)

struct idt_entry
{
	unsigned int
		base_00_16 : 16,
		sel : 16,
		always0 : 8,
		type : 4,
		memory_segment : 1,
		priv : 2,
		present : 1,
		base_16_32 : 16;
} __attribute__((packed));

struct idt_ptr
{
	uint16_t limit;
	struct idt_entry *base;
} __attribute__((packed));

extern void idt_set_interrupt(uint8_t num, void (*base)(), uint8_t priv);
extern void idt_set_task_gate(uint8_t num, uint16_t sel, uint8_t priv);
extern void idt_install(void);
extern void init_syscalls(void);

#endif
