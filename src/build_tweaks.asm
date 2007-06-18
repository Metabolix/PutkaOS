[BITS 32]

global __stack_chk_fail

__stack_chk_fail:
	jmp ret_0

ret_0:
	xor eax, eax
	ret
