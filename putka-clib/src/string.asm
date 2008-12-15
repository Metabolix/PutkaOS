BITS 32
SECTION .text

global memset

memset:
	push edi
	; address
	mov edi, [esp+8]

	; byte
	xor eax, eax
	movzx eax, BYTE[esp+12]
	mov edx, 0x01010101
	imul edx

	; count
	mov edx, [esp+16]

	mov ecx, edx
	shr ecx, 2
	rep stosd

	mov ecx, edx
	and ecx, 3
	rep stosb
	pop edi
	ret
