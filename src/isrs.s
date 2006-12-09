;extern active_thread_ptr
extern isr_handler

%macro isr_0_7 1
global isr%1
isr%1:
	push DWORD 0
	push DWORD %1
	jmp isr_common
%endmacro

%macro isr_8_31 1
global isr%1
isr%1:
	push DWORD %1
	jmp isr_common
%endmacro

isr_common:
	pushad
	push ds
	push es
	push fs
	push gs

	; pino talteen
	;mov eax, [active_thread_ptr]
	;mov [eax], esp
	;mov [eax+4], ss

	; kutsutaan
	mov eax, esp
	push eax
	call isr_handler
	pop eax

	; pino takaisin
	;mov eax, [active_thread_ptr]
	;mov esp, [eax]
	;mov ss, [eax+4]

	pop gs
	pop fs
	pop es
	pop ds
	popad
	add esp, 8
	iret

isr_0_7 0
isr_0_7 1
isr_0_7 2
isr_0_7 3
isr_0_7 4
isr_0_7 5
isr_0_7 6
isr_0_7 7
isr_8_31 8
isr_8_31 9
isr_8_31 10
isr_8_31 11
isr_8_31 12
isr_8_31 13
isr_8_31 14
isr_8_31 15
isr_8_31 16
isr_8_31 17
isr_8_31 18
isr_8_31 19
isr_8_31 20
isr_8_31 21
isr_8_31 22
isr_8_31 23
isr_8_31 24
isr_8_31 25
isr_8_31 26
isr_8_31 27
isr_8_31 28
isr_8_31 29
isr_8_31 30
isr_8_31 31
