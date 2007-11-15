#include <idt.h>
#include <screen.h>

#include <sys/syscalls.list.h>
#include <panic.h>

extern void asm_syscall();

/**
* syscall_table: function pointers for asm to call.
**/
syscall_t syscall_table[0x100];
const syscall_t * const syscall_table_ptr = syscall_table;
const int syscall_table_size = (sizeof(syscall_table) / sizeof(syscall_t));

/**
* Out of range || not defined ; NOTICE: eax included in params!
**/
intptr_t syscall_illegal(uint_t eax, intptr_t ecx, intptr_t edx)
{
	kprintf("Illegal syscall (%01d, %01d, %01d) = (%02x, %02x, %02x) (pid %01d, tid %01d)",
		eax, ecx, edx, eax, ecx, edx,
		active_tid, active_pid);
	kill_thread(active_tid);
	return -1;
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

	#define SYSCALL_MACRO(number, func, name, prototype, ...) \
	extern prototype; \
	if (set_syscall(number, (syscall_t) func) != 0) { \
		goto paniikki; \
	}
	#include <sys/syscalls.list.h>

	#undef SYSCALL_MACRO

	idt_set_interrupt(IDT_SYSCALL, asm_syscall, IDT_PRIV_USER);
	return;

paniikki:
	panic("init_syscalls: Liian suuria syscall-lukuja!");
}
