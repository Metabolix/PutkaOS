BITS 32
global start_idle_thread
extern active_process
extern debug_print_num

start_idle_thread:
	call debug_print_num
	call debug_print_num
	cli
	mov eax, [ebx+12]    ; process_id
	mov ebx, [esp+8]     ; process_id_t *
	mov [ebx], eax       ; active_process
	mov ebx, [esp+4]     ; thread *
	mov eax, [ebx+4]     ; ss
	mov ecx, [ebx+8]     ; stack
	call debug_print_num
	mov ss, eax
	mov esp, ecx
	call debug_print_num
	pop gs
	pop fs
	pop es
	pop ds
	pop ss
	popad
	call debug_print_num
	add esp, 8
	call debug_print_num
	sti
	call debug_print_num
	iret
