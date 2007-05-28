BITS 32

%macro defall 1
%1
%1 f, DWORD, 4
%1 l, TWORD, 10
%endmacro

%macro log10m 0-3 ,QWORD,8
global log10%1
log10%1:
	fldlg2         ; push lg(2)
	fld %2 [esp+4] ; push param
	fyl2x          ; st0 = lb(st0) * st1
	ret
%endmacro

%macro powm 0-3 ,QWORD,8
global pow%1
pow%1:
	fldlg2
	fld %2 [esp+4+(0 * %3)]
	fld %2 [esp+4+(1 * %3)]
	f2xm1 ; st0 = 2**st0 - 1
	ret
%endmacro


defall log10m
defall powm
