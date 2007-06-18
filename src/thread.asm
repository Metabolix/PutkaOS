BITS 32
global start_idle_thread
global kernel_idle_loop

start_idle_thread:
	mov ebx, [esp+4]
	mov eax, [ebx+4]     ; ss
	mov ecx, [ebx]     ; stack
	mov esp, ecx
	mov ss, ax
	pop gs
	pop fs
	pop es
	pop ds
	pop ss
	popad
	add esp, 8
	sti
	iret


kernel_idle_loop:
	hlt
	jmp kernel_idle_loop
