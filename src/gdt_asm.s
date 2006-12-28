BITS 32
global gdt_flush
extern gdt_pointer

gdt_flush:
   lgdt [gdt_pointer]
   jmp 0x08:here
here:
   mov ax,0x10
   mov ds,ax
   mov es,ax
   mov fs,ax
   mov gs,ax
   mov ss,ax
   ret
