%macro irq 1
global irq%1
irq%1:
	pushad
	push %1 ;interrupt number, for irq_handler
	extern irq_handler
	call irq_handler
	pop eax ;clean stack
	popad
	iret ;return from interrupt
%endmacro

irq 0
irq 1

