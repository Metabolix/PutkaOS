global start_idle_thread
extern active_process

start_idle_thread:
	cli
	mov ecx, [esp+8]
	mov esp, [esp+4]
	mov eax, [esp+12]
	mov [ecx], eax
	mov eax, [esp+4]
	mov ss, eax
	mov esp, [esp]
	pop gs
	pop fs
	pop es
	pop ds
	popad
	add esp, 8
	sti
	iret
