BITS 32

global asm_sti
global asm_cli
global asm_int
global asm_hlt
global asm_nop
global asm_ret
global asm_ud0
global asm_hlt_until_true

global asm_get_cr0
global asm_get_cr2
global asm_get_cr3

global asm_set_cr0
global asm_set_cr3

global asm_flush_cr3
global asm_invlpg

global asm_idt_load
global asm_gdt_flush

global asm_set_tr
global asm_get_tr

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
asm_ret:
	ret
asm_ud0:
	ud0
	ret

asm_invlpg:
	mov eax, [esp+4]
	invlpg [eax]
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

asm_gdt_flush:
	mov eax, [esp+4]
	lgdt [eax]
	mov ax,0x88
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax
	jmp 0x80:.return
.return
	ret

asm_idt_load:
	mov eax, [esp+4]
	lidt [eax]
	xor eax, eax
	ret

asm_set_tr:
	mov ax, [esp+4]
	ltr ax
	ret

asm_get_tr:
	xor eax, eax
	str ax
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

asm_flush_cr3:
	mov eax, cr3
	nop
	mov cr3, eax
	xor eax, eax
	ret

asm_int:
	xor eax, eax
	mov al, [esp+4]
	mul BYTE [.table_entry_size]
	add eax, .table_begin
	jmp eax

.table_entry_size:
	db ((.table_end - .table_begin) / 256)

.table_begin:
%assign i 0
%rep    256
	int i
	ret
%assign i i+1
%endrep
.table_end:
	nop

