[BITS 32]

extern syscall_table_ptr
extern syscall_table_size

global asm_syscall
global make_syscall

make_syscall:
	push ebx ; esp -= 4
	mov eax, [esp+8]
	mov ebx, [esp+12]
	mov ecx, [esp+16]
	mov edx, [esp+20]
	int 0x80
	pop ebx
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
	enter 0, 16

	; parametrit; int f(eax, ebx, ecx, edx);
	mov [esp+3*4], edx
	mov [esp+2*4], ecx
	mov [esp+1*4], ebx
	mov [esp+0*4], eax
	mov edx,[syscall_table_ptr]
	lea ecx,[edx+4*eax]
	call [ecx]

	leave
	iret

.user_syscall:
	push ebp
	mov ebp, esp
	mov esp, [ebp+16]
	sub esp, 24

	mov [esp+4*4 + 3*2], ds
	mov [esp+4*4 + 2*2], es
	mov [esp+4*4 + 1*2], fs
	mov [esp+4*4 + 0*2], gs

	; parametrit; int f(eax, ebx, ecx, edx);
	mov [esp+3*4], edx
	mov [esp+2*4], ecx
	mov [esp+1*4], ebx
	mov [esp+0*4], eax
	mov edx,[syscall_table_ptr]
	lea ecx,[edx+4*eax]

	mov ax, ss
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call [ecx]

	mov gs, [esp + 4*4 + 0*2]
	mov fs, [esp + 4*4 + 1*2]
	mov es, [esp + 4*4 + 2*2]
	mov ds, [esp + 4*4 + 3*2]

	leave
	iret
