BITS 32

global asm_float_div_by_zero
global asm_float_invalid

asm_float_div_by_zero:
	fld1
	fldz
	fdivp st1
	ffree st0
	fincstp
	ret

asm_float_invalid:
	; TODO
	ret

%macro defall 1
%1
%1 f, DWORD, 4
%1 l, TWORD, 10
%endmacro

%macro frexp_m 0-3 ,QWORD,8
global frexp%1
frexp%1:
	fld %2 [esp+4]
	mov eax, [esp+4+%3]
	fxtract
	fxch st0, st1
	fld1
	faddp st1
	fstp DWORD[eax]
	fld1
	fadd st0
	fdivp st1
	ret
%endmacro

%macro log10_m 0-3 ,QWORD,8
global log10%1
log10%1:
	fldlg2         ; push lg(2)
	fld %2 [esp+4] ; push param
	fyl2x          ; st0 = lb(st0) * st1
	ret
%endmacro

extern kprintf
kaksteksti:
	db "%08x & %08x ...",10,13,0
teksti:
	db " ** ecx: %08x",10,13,0

%macro exp2_m 0-3 ,QWORD,8
global exp2%1
exp2%1:
	fld %2 [esp+4] ; push param
	sub esp, 8

	fxtract

	fxch st0, st1
	fist DWORD[esp]
	pop ecx ; oikea
	fchs
	fistp DWORD[esp]
	pop edx ; käännetty

	fld1
	fsubp st1

	f2xm1

	fld1
	faddp st1

	fadd st0

	push edx
	push ecx
	push kaksteksti
	call kprintf
	pop eax
	pop ecx
	pop edx

	cmp ecx, edx
	jne .jatkuu
	ret
.jatkuu
	xor eax, eax
	inc eax
	rcr eax, 1
	test ecx, eax
	jz .pos

	mov ecx, edx
.neg:
	fsqrt
	dec ecx
	jnz .neg
	ret
.pos:
	fmul st0
	dec ecx
	jnz .pos
	ret
%endmacro

; 2 ** (a+b)

%macro pow_common_m 0-3 ,QWORD,8
global pow_common%1
pow_common%1: ; pow(x,y) = x**y
	fld %2 [esp+4+(0 * %3)] ; push x
	fld %2 [esp+4+(1 * %3)] ; push y
	jmp pow_base
%endmacro

;;;;;;;; pow_base
pow_base:
	sub esp, 4
	xor eax, eax
	fyl2x     ; pop2 ; push y * lb(x)
	fxtract   ; pop  ; st0 => (m * 2**e) ; push e ; push m;

	; a**(-b) => 1/(a**b)
	fldz
	fcomip st1
	jc .suuri_on
	inc eax
	fchs
.suuri_on:
	fld1      ; push 1.0
	fsubp st1 ; st0 = {st0 + st1} = 2**m
	f2xm1     ; st0 = 2**m - 1
	fld1      ; push 1.0
	faddp st1 ; st0 = {st0 + st1} = 2**m
	fadd st0  ; st0 = {st0 + st1} = 2**m

	; a**(-b) => 1/(a**b)
	test eax, eax
	jz .suuri_oli
	fld1
	fxch st1, st0
	fdivp st1

.suuri_oli:
	fxch st1, st0
	fistp DWORD[esp]
	pop ecx

	; st0 = st0**(2**eax)
	xor eax, eax
	cmp ecx, eax
	jz .ret_0
	jl .neg
.pos:
	dec ecx
	fmul st0
	jnz .pos
	ret
.neg:
	inc ecx
	fsqrt
	jnz .neg
	ret
.ret_0:
	ffree st0
	fincstp
	fldz
	ret
;;;;;;;; END OF pow_base

defall log10_m
defall pow_common_m
defall frexp_m
defall exp2_m
