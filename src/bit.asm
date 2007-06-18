BITS 32
;extern unsigned int set_bit(unsigned int num, int bit, int value);
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
	mov edx, [esp+12]
	test edx, edx
	mov edx, [esp+4]
	jz nollaksi
	mov eax, 1
	shl eax, cl
	or eax, edx
	ret
nollaksi:
	mov eax, 0xfffffffe
	rol eax, cl
	and eax, edx
	ret

;extern unsigned int get_bit(unsigned int num, int bit);
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

;extern void taikatemppu();
global taikatemppu
taikatemppu:
	ret
