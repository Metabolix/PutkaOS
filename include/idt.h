#ifndef _IDT_H
#define _IDT_H 1

#include <stdint.h>

#define IDT_PRESENT (0x80)
#define IDT_32_BIT_INTGATE (0x0E)

#define IDT_TYPICAL_FLAGS (IDT_PRESENT | IDT_32_BIT_INTGATE)
#define IDT_TYPICAL_CS (0x08)

struct idt_entry
{
	uint16_t base_lo;
	uint16_t sel;
	uint8_t always0;
	uint8_t flags;
	uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void idt_set_gate(uint8_t num, uint_t base, uint16_t sel, uint8_t flags);
extern void idt_install(void);
extern void init_syscalls(void);

#endif
