BITS 32
SECTION .text

global read_cmos

read_cmos:
	xor ecx, ecx
	mov edx, [esp+4]
	cli
.loop:
	mov al, cl
	out 0x70, al
	nop
	nop
	nop
	in al, 0x71
	mov [edx], al
	inc edx
	inc ecx
	cmp cl, 0x80
	jne .loop
	sti
	ret

