[BITS 32]

global ret_0

global __stack_chk_fail
global __fixunsxfdi

__stack_chk_fail:
__fixunsxfdi:
	; jmp ret_0 ; suoraan perässä on, turha hyppiä

ret_0:
	xor eax, eax
	ret
