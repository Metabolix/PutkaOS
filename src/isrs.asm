BITS 32
extern isr_handler

%macro isr_00_07 1
global isr%1
isr%1:
	push DWORD 0
	push DWORD %1
	jmp isr_common
%endmacro

%macro isr_08_1f 1
global isr%1
isr%1:
	push DWORD %1
	jmp isr_common
%endmacro

isr_common:
	push ax
	push cx
	mov ax, ss
	mov cx, ds
	cmp ax, cx
	pop cx
	pop ax
	jne .user_isr
.kernel_isr:
	pushad
	push ss
	push ds
	push es
	push fs
	push gs

	; kutsutaan
	mov eax, esp
	push eax
	call isr_handler
	pop eax

	pop gs
	pop fs
	pop es
	pop ds
	pop ss
	popad
	add esp, 8
	iret

.user_isr:
	push ebp
	mov ebp, esp
	mov esp, [ebp + 4*6]

	push DWORD [ebp + 4*7] ; ss (iret)
	push DWORD [ebp + 4*6] ; esp (iret)
	push DWORD [ebp + 4*5] ; eflags
	push DWORD [ebp + 4*4] ; cs
	push DWORD [ebp + 4*3] ; eip
	push DWORD [ebp + 4*2] ; error code
	push DWORD [ebp + 4*1] ; isr num

	pushad
	; manipuloidaan ebp (esp onkin jo oikein)
	mov eax, [ebp]
	mov [esp + 4*3], eax

	; push ss
	mov eax, [ebp + 4*7]
	push eax
	; /push ss
	push ds
	push es
	push fs
	push gs

	; kutsutaan
	mov eax, esp
	push eax
	call isr_handler
	pop eax

	pop gs
	pop fs
	pop es
	pop ds
	; pop ss
	pop eax
	mov [esp + 4*8 + 4*6], eax
	; /pop ss
	popad

	;leave
	add esp, 8
	iret

isr_00_07 0x00
isr_00_07 0x01
isr_00_07 0x02
isr_00_07 0x03
isr_00_07 0x04
isr_00_07 0x05
isr_00_07 0x06
isr_00_07 0x07

;isr_08_1f 0x08 ; Double fault
isr_08_1f 0x09
isr_08_1f 0x0a
isr_08_1f 0x0b
isr_08_1f 0x0c
isr_08_1f 0x0d
;isr_08_1f 0x0e ; Page fault
isr_08_1f 0x0f
isr_08_1f 0x10
isr_08_1f 0x11
isr_08_1f 0x12
isr_08_1f 0x13
isr_08_1f 0x14
isr_08_1f 0x15
isr_08_1f 0x16
isr_08_1f 0x17
isr_08_1f 0x18
isr_08_1f 0x19
isr_08_1f 0x1a
isr_08_1f 0x1b
isr_08_1f 0x1c
isr_08_1f 0x1d
isr_08_1f 0x1e
isr_08_1f 0x1f
