BITS 32
SECTION .text

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
%1 l, TWORD, 12
%endmacro

;; int asm_is_integer (F f)
;; ret ([f] == f)

%macro is_integer_m 0-3 ,QWORD,8
global asm_is_integer%1
asm_is_integer%1:
	xor eax, eax
	fld %2 [esp+4+(0 * %3)] ; push x
	fld st0
	frndint
	fcomip st1
	ffreep st0
	jne .ret
	inc eax
.ret:
	ret
%endmacro

;; int asm_is_odd_integer (F f)
;; ret ([f] == f && ([f] % 2 == 0))

%macro is_odd_integer_m 0-3 ,QWORD,8
global asm_is_odd_integer%1
asm_is_odd_integer%1:
	fld %2 [esp+4+(0 * %3)] ; push x
	fld st0
	frndint
	fcomip st1
	jne .not_int
	fistp QWORD[esp-8]
	mov eax, [esp-8]
	and eax, 1
	ret
.not_int:
	xor eax, eax
	ffreep st0
	ret

%endmacro

;; F asm_fabs (F f)
;; ret |f|

%macro fabs_m 0-3 ,QWORD,8
global asm_fabs%1
asm_fabs%1:
	fld %2 [esp+4+(0 * %3)] ; push x
	fabs
	ret
%endmacro

;; F asm_frndint (F f)
;; [x]

%macro frndint_m 0-3 ,QWORD,8
global asm_frndint%1
asm_frndint%1:
	fld %2 [esp+4+(0 * %3)] ; push x
	frndint
	ret
%endmacro

;; F asm_fchs (F f)
;; ret -f

%macro fchs_m 0-3 ,QWORD,8
global asm_fchs%1
asm_fchs%1:
	fld %2 [esp+4+(0 * %3)] ; push x
	fchs
	ret
%endmacro

;; F asm_atan2 (F y, F x)
;; tan(ret) = y / x; sin(ret) = ay, cos(ret) = ax, a > 0

%macro atan2_m 0-3 ,QWORD,8
global asm_atan2%1
asm_atan2%1:
	fld %2 [esp+4+(0 * %3)] ; push y
	fld %2 [esp+4+(1 * %3)] ; push x
	fpatan
	ret
%endmacro

;; F asm_atan (F y, F x)
;; tan(ret) = y / x;

%macro atan_m 0-3 ,QWORD,8
global asm_atan%1
asm_atan%1:
	fld %2 [esp+4+(0 * %3)] ; push y
	fld %2 [esp+4+(1 * %3)] ; push x
	jmp asm_atan_base
%endmacro

asm_atan_base:
	fldz
	fcomip st1
	jl .jatka
	fchs
	fxch st1
	fchs
	fxch st1
.jatka:
	fpatan
	ret

;; F asm_asin (F x)
;; sin(ret) = x;

%macro asin_m 0-3 ,QWORD,8
global asm_asin%1
asm_asin%1:
	fld %2 [esp+4+(0 * %3)] ; push x
	fld1
	fld st1
	fmul st0
	fsubp st1
	fsqrt
	fpatan
	ret
%endmacro

;; F asm_acos (F x)
;; cos(ret) = x;

%macro acos_m 0-3 ,QWORD,8
global asm_acos%1
asm_acos%1:
	fld %2 [esp+4+(0 * %3)] ; push x
	fld1
	fld st1
	fmul st0
	fsubp st1
	fsqrt
	fxch st1
	fpatan
	ret
%endmacro

;; F asm_tan (F f)
;; ret tan(f)

%macro tan_m 0-3 ,QWORD,8
global asm_tan%1
asm_tan%1:
	fld %2 [esp+4+(0 * %3)]
	fptan
	ret
%endmacro

;; F asm_sin (F f)
;; ret sin(f)

%macro sin_m 0-3 ,QWORD,8
global asm_sin%1
asm_sin%1:
	fld %2 [esp+4+(0 * %3)]
	fsin
	ret
%endmacro

;; F asm_cos (F f)
;; ret sin(f)

%macro cos_m 0-3 ,QWORD,8
global asm_cos%1
asm_cos%1:
	fld %2 [esp+4+(0 * %3)]
	fcos
	ret
%endmacro

;; F asm_sqrt (F f)
;; ret sqrt(f)

%macro sqrt_m 0-3 ,QWORD,8
global asm_sqrt%1
asm_sqrt%1:
	fld %2 [esp+4+(0 * %3)]
	fsqrt
	ret
%endmacro

;; F asm_hypot (F y, F x)
;; ret sqrt(x^2 + y^2)

%macro hypot_m 0-3 ,QWORD,8
global asm_hypot%1
asm_hypot%1:
	fld %2 [esp+4+(0 * %3)] ; push y
	fmul st0
	fld %2 [esp+4+(1 * %3)] ; push x
	fmul st0
	faddp st1
	fsqrt
	ret
%endmacro

;; int asm_fxam (F f)
;; ret fstsw ax

%macro fxam_m 0-3 ,QWORD,8
global asm_fxam%1
asm_fxam%1:
	fld %2 [esp+4+(0 * %3)]
	fxam
	ffreep st0
	xor eax, eax
	fstsw ax
	ret
%endmacro

;; F asm_fxtract (F f, int *exp)
;; f = ret * 2^(*exp)

%macro fxtract_m 0-3 ,QWORD,8
global asm_fxtract%1
asm_fxtract%1:
	fld %2 [esp+4+(0 * %3)]
	lea eax, [esp+4+(1 * %3)]
	fxtract
	fxch st1
	fistp DWORD[eax]
	ret
%endmacro

;; F asm_fyl2x (F y, F x)
;; ret y * log2(x)

%macro fyl2x_m 0-3 ,QWORD,8
global asm_fyl2x%1
asm_fyl2x%1:
	fld %2 [esp+4+(0 * %3)] ; push y
	fld %2 [esp+4+(1 * %3)] ; push x
	fyl2x
	ret
%endmacro

;; F asm_fyl2x (F y, F x), |x| < 1 - sqrt(0.5)
;; ret y * log2(x+1)

%macro fyl2xp1_m 0-3 ,QWORD,8
global asm_fyl2xp1%1
asm_fyl2xp1%1:
	fld %2 [esp+4+(0 * %3)] ; push y
	fld %2 [esp+4+(1 * %3)] ; push x
	fyl2xp1
	ret
%endmacro

;; F asm_log2 (F x)
;; ret log2(x)

%macro log2_m 0-3 ,QWORD,8
global asm_log2%1
asm_log2%1:
	fld1           ; push 1
	fld %2 [esp+4] ; push param
	fyl2x          ; st0 = lb(st0) * st1
	ret
%endmacro

;; F asm_log10 (F x)
;; ret log10(x)

%macro log10_m 0-3 ,QWORD,8
global asm_log10%1
asm_log10%1:
	fldlg2         ; push lg(2)
	fld %2 [esp+4] ; push param
	fyl2x          ; st0 = lb(st0) * st1
	ret
%endmacro

;; F asm_ln (F x)
;; ret ln(x)

%macro ln_m 0-3 ,QWORD,8
global asm_ln%1
asm_ln%1:
	fldln2         ; push ln(2)
	fld %2 [esp+4] ; push param
	fyl2x          ; st0 = lb(st0) * st1
	ret
%endmacro

;; F asm_exp2 (F x)
;; ret 2^(x)

%macro exp2_m 0-3 ,QWORD,8
global asm_exp2%1
asm_exp2%1:
	fld %2 [esp+4] ; push param
	jmp asm_exp2_base
%endmacro

asm_exp2_base:
	; fld x
	; sub esp, 8

	; TODO:
	; C:ssä tarkistetaan inf yms ongelmat...
	fld st0
	fabs
	; jos |x| < 1
		; suoraan f2xm1 + 1
	; muuten
		; fscale(f2xm1(x - [x]) + 1, [x])

	ret

;; F asm_exp (F x)
;; ret e^(x) = 2^(x lb e)

%macro exp_m 0-3 ,QWORD,8
global asm_exp%1
asm_exp%1:
	fld %2 [esp+4] ; push param
	fldl2e
	fmulp st1
	jmp asm_exp2_base
%endmacro

;; F asm_expm1 (F x)
;; ret e^(x) - 1 = 2^(x lb e) - 1
;; TODO: tarkkuus!

%macro expm1_m 0-3 ,QWORD,8
global asm_expm1%1
asm_expm1%1:
	fld %2 [esp+4] ; push param
	fldl2e
	fmulp st1
	call asm_exp2_base
	fld1
	fsubp st1
	ret
%endmacro

;; F asm_frexp (F x, int *exp)
;; x = ret * 2^(*exp)

%macro frexp_m 0-3 ,QWORD,8
global asm_frexp%1
asm_frexp%1:
	fld %2 [esp+4]
	mov eax, [esp+4+%3]
	jmp asm_frexp_base
%endmacro

asm_frexp_base:
	; fld x
	; mov eax, [esp + 4 + sizeof(x)]
	test eax, eax
	jnz .ei_suojella
	lea eax, [esp-4] ; Suojataan segfaultilta
.ei_suojella:
	fxtract
	fxch st0, st1
	fld1
	faddp st1
	fistp DWORD[eax]
	fld1
	fadd st0
	fdivp st1
	ret

;; F asm_logb (F x)
;; 2^(ret) <= x < 2^(ret+1)

%macro logb_m 0-3 ,QWORD,8
global asm_logb%1
asm_logb%1:
	fld %2 [esp+4]
%endmacro

asm_logb_base:
	; fld param
	fxtract
	fld1
	fstp st1
	fsubp st1
	ret

;; I asm_ilogb (F x)
;; 2^(ret) <= x < 2^(ret+1)

%macro ilogb_m 0-3 ,QWORD,8
global asm_ilogb%1
asm_ilogb%1:
	fld %2 [esp+4]
	call asm_logb_base
	fistp DWORD[esp-4]
	mov eax, [esp-4]
	ret
%endmacro

;; F asm_fscalei (F f, int e)
;; ret f * 2^([e])

%macro fscalei_m 0-3 ,QWORD,8
global asm_fscalei%1
asm_fscalei%1:
	fild DWORD[esp+4+%3]
	fld %2 [esp+4]
	fscale
	fstp st1
	ret
%endmacro

%macro fscalel_m 0-3 ,QWORD,8
global asm_fscalel%1
asm_fscalel%1:
	jmp asm_fscalei%1
%endmacro

;; DEFS

defall is_integer_m
defall is_odd_integer_m
defall fabs_m
defall frndint_m
defall fchs_m
defall atan2_m
defall atan_m
defall asin_m
defall acos_m
defall tan_m
defall sin_m
defall cos_m
defall sqrt_m
defall hypot_m
defall fxam_m
defall fxtract_m
defall fyl2x_m
defall fyl2xp1_m
defall log2_m
defall log10_m
defall ln_m
defall exp2_m
defall exp_m
defall expm1_m
defall frexp_m
defall logb_m
defall ilogb_m
defall fscalei_m
defall fscalel_m
