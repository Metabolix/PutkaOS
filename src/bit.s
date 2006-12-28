BITS 32
global set_bit
set_bit:
	mov ecx, [esp+8]
	mov edx, ecx
	and edx, 0xffffffe0
	cmp edx, 0
	je set_bit_jatka
	mov eax, [esp+4]
	ret
set_bit_jatka:
	mov edx, [esp+4]
	cmp DWORD[esp+12], 0
	je nollaksi
	mov eax, 1
	shl eax, cl
	or eax, edx
	ret
nollaksi:
	mov eax, 0xfffffffe
	rol eax, cl
	and eax, edx
	ret

global get_bit
get_bit:
	mov ecx, [esp+8]
	mov edx, ecx
	and edx, 0xffffffe0
	jz get_bit_jatka
	xor eax, eax
	ret
get_bit_jatka:
	mov eax, [esp+4]
	shr eax, cl
	and eax, 1
	ret
