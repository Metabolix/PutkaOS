BITS 32
global asm_sti
global asm_cli
global asm_hlt
global asm_nop
global asm_hlt_until_true

global asm_get_cr0
global asm_get_cr2
global asm_get_cr3

global asm_set_cr0
global asm_set_cr3

global asm_idt_load

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
.loop:
	mov dl, [eax]
	test dl, dl
	jnz .ret
	hlt
	jmp .loop
.ret:
	ret

asm_get_cr0:
	mov eax, cr0
	ret

asm_get_cr2:
	mov eax, cr2
	ret

asm_get_cr3:
	mov eax, cr3
	ret

asm_set_cr0:
	mov eax, [esp+4]
	mov cr0, eax
	ret

asm_set_cr3:
	mov eax, [esp+4]
	mov cr3, eax
	ret

asm_idt_load:
	mov eax, [esp+4]
	lidt [eax]
	xor eax, eax
	ret
