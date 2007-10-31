BITS 32
extern active_thread
extern irq_handler
global start_threading

%macro irq 1
global irq%1
irq%1:
	cld
	cli
	push DWORD 0
	push DWORD %1
	jmp irq_handler_common
%endmacro

irq_handler_common:
	pushad
	push ss
	push ds
	push es
	push fs
	push gs

	mov eax, [active_thread]
	test eax, eax
	jz irq_handler_common_no_thread

	mov [eax], esp
	mov [eax+4], ss
	mov ecx, esp

	extern _sys_stack
	mov esp, _sys_stack
	mov eax, 0x10
	mov ss, eax
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax

	push ecx
	call irq_handler
	pop ecx
start_threading:
	mov eax, [active_thread]
	mov esp, [eax]
	mov ss, [eax+4]

	pop gs
	pop fs
	pop es
	pop ds
	pop ss
	popad
	add esp, 8
	sti
	iret

irq_handler_common_no_thread:
	mov eax, esp
	push eax
	call irq_handler
	pop eax

	pop gs
	pop fs
	pop es
	pop ds
	pop ss
	popad
	add esp, 8
	sti
	iret

irq 0
irq 1
irq 2
irq 3
irq 4
irq 5
irq 6
irq 7
irq 8
irq 9
irq 10
irq 11
irq 12
irq 13
irq 14
irq 15
