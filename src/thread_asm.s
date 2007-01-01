BITS 32
global start_idle_thread
extern active_thread_ptr

extern debug_point
extern debug_print_int

start_idle_thread:
	mov ebx, [active_thread_ptr]
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
