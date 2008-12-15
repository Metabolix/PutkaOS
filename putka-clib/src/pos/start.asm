BITS 32
SECTION .text

extern main
global _start

_start:
	call main
	int 0x40
