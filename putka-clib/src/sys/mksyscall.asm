[BITS 32]

global mksyscall

mksyscall:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	int 0x80
	ret
