[BITS 32]

extern syscall_table_ptr
extern syscall_table_size
extern syscall_illegal

global asm_syscall
global make_syscall

make_syscall:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	int 0x80
	ret

asm_syscall:
	sti
	; kelvollinen syscall
	cmp eax, 0
	jle .out_of_range
	cmp eax, [syscall_table_size]
	jge .out_of_range
	jmp .prepare_call

.out_of_range:
	push eax
	call syscall_illegal
	add esp, 4
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
	enter 0, 12

	; parametrit; int f(eax, ecx, edx);
	mov [esp+2*4], edx
	mov [esp+1*4], ecx
	mov [esp+0*4], eax
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
	sub esp, 20

	mov [esp + 3*4 + 3*2], ds
	mov [esp + 3*4 + 2*2], es
	mov [esp + 3*4 + 1*2], fs
	mov [esp + 3*4 + 0*2], gs

	; parametrit; int f(eax, ecx, edx);
	mov [esp + 2*4], edx
	mov [esp + 1*4], ecx
	mov [esp + 0*4], eax
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

	mov gs, [esp + 3*4 + 0*2]
	mov fs, [esp + 3*4 + 1*2]
	mov es, [esp + 3*4 + 2*2]
	mov ds, [esp + 3*4 + 3*2]

	leave
	iret

.user_illegal_syscall:
	push eax
	call syscall_illegal
	leave
	iret
