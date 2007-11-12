[BITS 32]
extern kmain

global stack_for_page_fault
global stack_for_double_fault
global stack_for_hw_int_task
global stack_for_hw_interrupt

global start
start:
	; Väliaikainen pino, tätähän ei pitäisi kenenkään tarvita. ;)
	mov esp, stack_for_double_fault
	push ebx
	jmp main_caller

ALIGN 4
mboot:
	MULTIBOOT_PAGE_ALIGN	equ 1<<0
	MULTIBOOT_MEMORY_INFO	equ 1<<1
	; MULTIBOOT_AOUT_KLUDGE	equ 1<<16
	MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
	MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO ;| MULTIBOOT_AOUT_KLUDGE
	MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
	EXTERN code, bss, end

	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM

main_caller:
    call kmain
    jmp $

SECTION .bss
stack_for_:
	resb 0x1000
stack_for_page_fault:
	resb 0x1000
stack_for_double_fault:
	resb 0x1000
stack_for_hw_int_task:
	resb 0x100
stack_for_hw_interrupt:
