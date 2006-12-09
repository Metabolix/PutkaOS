global get_regs
get_regs:
	push eax
	mov eax, esp
	mov esp, [esp+8]
	add esp, 76 ; 19*4
	push ss
	push esp
	pushfd
	push cs
	push DWORD 0
	push DWORD 0
	push DWORD 0
	pushad
	push ds
	push es
	push fs
	push gs
	mov esp, eax
	pop eax
	ret
