BITS 32
SECTION .text

extern irq_handlers
extern irq_handling
extern irq_illegal_handler
extern kernel_tasks
extern scheduler
extern thread_ending

global irq_task
global irq_common_ret
global isr_scheduler
global isr_thread_ending

%macro irq 1
global irq%1
irq%1:
	cld
	cli
	mov [irq_handling], DWORD %1
	jmp irq_common
%endmacro

irq_common:
	push eax
	mov eax, kernel_tasks
	mov [eax+4*8], DWORD irq_task
	pop eax
	call 0x08:0 ; HW INT TSS
irq_common_ret:
	mov [irq_handling], DWORD -1
	sti
	iret

irq_task:
	mov ebx, [irq_handling]
	test ebx, 0xfffffff0
	jnz .illegal
	mov edx, irq_handlers
	lea eax, [edx + 4*ebx]
	call [eax]
	call scheduler
	test bl, 0x08
	jnz .ge8
.lt8:
	mov al, 0x20
	movzx dx, al
	out dx, al
	iret
.ge8:
	mov al, 0xA0
	movzx dx, al
	out dx, al
	iret
.illegal:
	call irq_illegal_handler
	iret

irq_scheduler_only:
	call scheduler
	iret

isr_scheduler:
	cld
	cli
	mov [irq_handling], DWORD 0x100
	push eax
	mov eax, kernel_tasks
	mov [eax+4*8], DWORD irq_scheduler_only
	pop eax
	call 0x08:0 ; HW INT TSS
	mov [irq_handling], DWORD -1
	sti
	iret

isr_thread_ending:
	mov ax, ss
	mov cx, ds
	cmp ax, cx
	jne .user_thread
.kernel_thread:
	jmp thread_ending
.user_thread:
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov esp, [esp + 4*3]
	jmp thread_ending

irq 0x00
irq 0x01
irq 0x02
irq 0x03
irq 0x04
irq 0x05
irq 0x06
irq 0x07
irq 0x08
irq 0x09
irq 0x0a
irq 0x0b
irq 0x0c
irq 0x0d
irq 0x0e
irq 0x0f
