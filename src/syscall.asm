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
	push ebx
	; kelvollinen syscall
	cmp eax, 0
	jl .out_of_range
	cmp eax, [syscall_table_size]
	jge .out_of_range

	; parametrit; int f(eax, ebx, ecx, edx);
	push edx
	push ecx
	push ebx
	push eax
	mov edx,[syscall_table_ptr]
	lea ebx,[edx+4*eax]
	call [ebx]
	add esp, 16
	pop ebx
	iret

.out_of_range:
	xor eax, eax
	dec eax
	pop ebx
	iret
