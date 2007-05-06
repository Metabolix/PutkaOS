global asm_sti
global asm_cli
global asm_hlt
global asm_nop
global asm_hlt_until_true

asm_sti:
	sti
	ret
asm_cli:
	cli
	ret
asm_hlt:
	hlt
	ret
asm_nop:
	nop
	ret

asm_hlt_until_true:
	mov eax, [esp+4]
asm_hlt_until_true_loop:
	mov dl, [eax]
	test dl, dl
	jnz asm_hlt_until_true_ret
	hlt
	jmp asm_hlt_until_true_loop
asm_hlt_until_true_ret:
	ret
