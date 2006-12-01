%macro irq 1
global irq%1
irq%1:
	cli
	pushad
	push %1 ;interrupt number, for irq_handler
	extern irq_handler
	call irq_handler
	pop eax ;clean stack
	popad
	sti
	iret ;return from interrupt
%endmacro

irq 0
irq 1
irq 2
irq 3
irq 4
irq 5
irq 6
irq 7
irq 8
irq 9
irq 10
irq 11
irq 12
irq 13
irq 14
irq 15
