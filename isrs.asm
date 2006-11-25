%macro isr 1
global isr%1
isr%1:
	;cli ;disable interrupts
	pushad
	push %1 ;interrupt number
	extern isr_handler
	call isr_handler
	pop eax ;clean stack
	popad
	;sti ;enable interrupts
	iret ;return from interrupt
%endmacro

isr 0
isr 1
isr 2
isr 3
isr 4
isr 5
isr 6
isr 7
isr 8
isr 9
isr 10
isr 11
isr 12
isr 13
isr 14
isr 15
isr 16
isr 17
isr 18
isr 19
isr 20
isr 21
isr 22
isr 23
isr 24
isr 25
isr 26
isr 27
isr 28
isr 29
isr 30
isr 31
