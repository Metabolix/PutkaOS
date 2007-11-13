#include <idt.h>
#include <screen.h>

#include <syscall/syscall.h>
#include <panic.h>

extern void asm_syscall();

/**
* syscall_table: function pointers for asm to call.
**/
syscall_t syscall_table[0x100];
const syscall_t * const syscall_table_ptr = syscall_table;
const int syscall_table_size = (sizeof(syscall_table) / sizeof(syscall_t));

/**
* Out of range || not defined
**/
intptr_t syscall_illegal(uint_t eax, intptr_t ecx, intptr_t edx)
{
	kprintf("Illegal syscall %01d (%02x) (pid %01d, tid %01d)", eax, eax, active_tid, active_pid);
	kill_thread(active_tid);
	return -1;
}

/**
* syscall_print: print(ecx);
**/
intptr_t syscall_print(uint_t eax, intptr_t ecx, intptr_t edx)
{
	print((const char *) ecx);
	return 0;
}

/**
* set_syscall: asetetaan syscall (eax = i) funktioon f.
**/
static int set_syscall(uint_t i, syscall_t f)
{
	if (!i) {
		return 0;
	}
	if (i >= syscall_table_size) {
		return -1;
	}
	syscall_table[i] = f;
	return 0;
}

/**
* init_syscalls: IDT ja kutsutaulu
**/
void init_syscalls(void)
{
	memset(syscall_table, 0, sizeof(syscall_table));

	#define SYSCALL_MACRO(number, func, ...) \
	extern intptr_t func(uint_t eax, intptr_t ecx, intptr_t edx); \
	if (set_syscall(number, func) != 0) { \
		goto paniikki; \
	}
	#include <syscall/syscalls.list.h>

	#undef SYSCALL_MACRO

	idt_set_interrupt(IDT_SYSCALL, asm_syscall, IDT_PRIV_USER);
	return;

paniikki:
	panic("init_syscalls: Liian suuria syscall-lukuja!");
}
