	.file	"irq.c"
.globl irq_handlers
	.bss
	.align 32
	.type	irq_handlers, @object
	.size	irq_handlers, 64
irq_handlers:
	.zero	64
	.section	.rodata
.LC0:
	.string	"IRQs remapped\n"
	.text
.globl irq_remap
	.type	irq_remap, @function
irq_remap:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	$33, (%esp)
	call	inportb
	movb	%al, -2(%ebp)
	movl	$161, (%esp)
	call	inportb
	movb	%al, -1(%ebp)
	movl	$17, 4(%esp)
	movl	$32, (%esp)
	call	outportb
#APP
	nop
#NO_APP
	movl	$17, 4(%esp)
	movl	$160, (%esp)
	call	outportb
#APP
	nop
#NO_APP
	movl	8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$33, (%esp)
	call	outportb
#APP
	nop
#NO_APP
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$161, (%esp)
	call	outportb
#APP
	nop
#NO_APP
	movl	$4, 4(%esp)
	movl	$161, (%esp)
	call	outportb
#APP
	nop
#NO_APP
	movl	$2, 4(%esp)
	movl	$33, (%esp)
	call	outportb
#APP
	nop
#NO_APP
	movl	$1, 4(%esp)
	movl	$33, (%esp)
	call	outportb
#APP
	nop
#NO_APP
	movl	$1, 4(%esp)
	movl	$161, (%esp)
	call	outportb
#APP
	nop
#NO_APP
	movsbl	-2(%ebp),%eax
	movl	%eax, 4(%esp)
	movl	$33, (%esp)
	call	outportb
	movsbl	-1(%ebp),%eax
	movl	%eax, 4(%esp)
	movl	$161, (%esp)
	call	outportb
	movl	$.LC0, %eax
	movl	%eax, (%esp)
	call	print
	leave
	ret
	.size	irq_remap, .-irq_remap
.globl install_irq
	.type	install_irq, @function
install_irq:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$40, 4(%esp)
	movl	$32, (%esp)
	call	irq_remap
	leave
	ret
	.size	install_irq, .-install_irq
	.section	.rodata
	.align 4
.LC1:
	.string	"Trying to install irq handler on irq, which doesn't exist!\n"
	.text
.globl install_irq_handler
	.type	install_irq_handler, @function
install_irq_handler:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	cmpl	$15, 8(%ebp)
	jg	.L6
	cmpl	$0, 8(%ebp)
	js	.L6
	movl	8(%ebp), %edx
	movl	12(%ebp), %eax
	movl	%eax, irq_handlers(,%edx,4)
	jmp	.L10
.L6:
	movl	$.LC1, %eax
	movl	%eax, (%esp)
	call	print
.L10:
	leave
	ret
	.size	install_irq_handler, .-install_irq_handler
	.section	.rodata
	.align 4
.LC2:
	.string	"Trying to uninstall irq handler number "
.LC3:
	.string	" but it doesn't exist\n"
	.align 4
.LC4:
	.string	"Trying to uninstall irq handler, out of indexes\n"
	.text
.globl uninstall_irq_handler
	.type	uninstall_irq_handler, @function
uninstall_irq_handler:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	cmpl	$15, 8(%ebp)
	jg	.L12
	cmpl	$0, 8(%ebp)
	js	.L12
	movl	8(%ebp), %eax
	movl	irq_handlers(,%eax,4), %eax
	testl	%eax, %eax
	jne	.L15
	movl	$.LC2, %eax
	movl	%eax, (%esp)
	call	print
	movl	8(%ebp), %eax
	andb	$90, %al
	addb	$48, %al
	movzbl	%al, %eax
	movl	%eax, (%esp)
	call	putch
	movl	8(%ebp), %eax
	andb	$10, %al
	addb	$48, %al
	movzbl	%al, %eax
	movl	%eax, (%esp)
	call	putch
	movl	$.LC3, %eax
	movl	%eax, (%esp)
	call	print
	jmp	.L19
.L15:
	movl	8(%ebp), %eax
	movl	$0, irq_handlers(,%eax,4)
	jmp	.L19
.L12:
	movl	$.LC4, %eax
	movl	%eax, (%esp)
	call	print
.L19:
	leave
	ret
	.size	uninstall_irq_handler, .-uninstall_irq_handler
	.section	.rodata
	.align 4
.LC5:
	.string	"Got interrupt, but we don't have handler for it!\n"
	.align 4
.LC6:
	.string	"PANIC: Irq_handler got irq which doesn't exist\n"
	.text
.globl irq_handler
	.type	irq_handler, @function
irq_handler:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	cmpl	$15, 8(%ebp)
	ja	.L21
	movl	8(%ebp), %eax
	movl	irq_handlers(,%eax,4), %eax
	testl	%eax, %eax
	je	.L23
	movl	8(%ebp), %eax
	movl	irq_handlers(,%eax,4), %eax
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	call	*%eax
	jmp	.L26
.L23:
	movl	8(%ebp), %eax
	andb	$90, %al
	addb	$48, %al
	movzbl	%al, %eax
	movl	%eax, (%esp)
	call	putch
	movl	8(%ebp), %eax
	andb	$10, %al
	addb	$48, %al
	movzbl	%al, %eax
	movl	%eax, (%esp)
	call	putch
	movl	$.LC5, %eax
	movl	%eax, (%esp)
	call	print
	jmp	.L26
.L21:
	movl	$.LC6, %eax
	movl	%eax, (%esp)
	call	print
#APP
	hlt
#NO_APP
.L26:
	cmpl	$7, 8(%ebp)
	jbe	.L27
	movl	$32, 4(%esp)
	movl	$160, (%esp)
	call	outportb
.L27:
	movl	$32, 4(%esp)
	movl	$32, (%esp)
	call	outportb
	leave
	ret
	.size	irq_handler, .-irq_handler
	.ident	"GCC: (GNU) 4.1.1 (Gentoo 4.1.1)"
	.section	.note.GNU-stack,"",@progbits
