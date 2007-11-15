[BITS 32]

extern syscall_table_ptr
extern syscall_table_size
extern syscall_illegal

global asm_syscall

asm_syscall:
	sti
	; kelvollinen syscall
	cmp eax, 0
	jle .out_of_range
	cmp eax, [syscall_table_size]
	jge .out_of_range
	jmp .prepare_call

.out_of_range:
	xor eax, eax
	dec eax
	iret

.prepare_call:
	; kernelin vai käyttäjän ohjelma?
	push ax
	push cx
	mov ax, ss
	mov cx, ds
	cmp ax, cx
	pop cx
	pop ax
	jne .user_syscall

.kernel_syscall:
	enter 0, 8

	; parametrit; int f(ecx, edx);
	mov [esp+1*4], edx
	mov [esp+0*4], ecx
	mov edx,[syscall_table_ptr]
	mov ecx,[edx+4*eax]
	test ecx, ecx
	jz .kernel_illegal_syscall
	call ecx
	leave
	iret

.kernel_illegal_syscall:
	push eax
	call syscall_illegal
	leave
	iret

.user_syscall:
	push ebp
	mov ebp, esp
	mov esp, [ebp+16]
	sub esp, 16

	mov [esp + 2*4 + 3*2], ds
	mov [esp + 2*4 + 2*2], es
	mov [esp + 2*4 + 1*2], fs
	mov [esp + 2*4 + 0*2], gs

	; parametrit; int f(ecx, edx);
	mov [esp + 1*4], edx
	mov [esp + 0*4], ecx
	mov edx,[syscall_table_ptr]
	mov ecx,[edx+4*eax]
	test ecx, ecx
	jz .user_illegal_syscall

	mov ax, ss
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call ecx

	mov gs, [esp + 2*4 + 0*2]
	mov fs, [esp + 2*4 + 1*2]
	mov es, [esp + 2*4 + 2*2]
	mov ds, [esp + 2*4 + 3*2]

	leave
	iret

.user_illegal_syscall:
	push eax
	call syscall_illegal
	leave
	iret
