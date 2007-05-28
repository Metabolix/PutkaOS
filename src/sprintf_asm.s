BITS 32

extern sprintf_start
extern sprintf_quit
extern fprintf
extern print

global sprintf

sprintf:
	mov eax,[esp]
	mov ecx,[esp+4]
	push eax
	push ecx
	call sprintf_start
	add esp, 8

	mov [esp+4], eax
	mov eax, sprintf_retpos
	mov [esp], eax
	jmp fprintf
sprintf_retpos:

	push eax
	mov ecx, esp
	mov edx, [esp+4]
	push eax
	push ecx
	push edx
	call sprintf_quit
	add esp, 8
	pop eax
	ret
