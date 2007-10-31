[BITS 32]

global memset

memset:
	; address
	mov edi, [esp+4]

	; byte
	xor eax, eax
	movzx eax, BYTE[esp+8]
	mov edx, 0x01010101
	imul edx

	; count
	mov edx, [esp+12]

	mov ecx, edx
	shr ecx, 2
	rep stosd

	mov ecx, edx
	and ecx, 3
	rep stosb
	ret
