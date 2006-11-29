nasm -f elf -o start.o start.asm
echo nasm start.o
nasm -f elf -o gdt_flush.o gdt.asm
echo nasm gdt_flush.o 
nasm -f elf -o irq.o irq.asm
echo nasm irq.o
nasm -f elf -o isrs.o isrs.asm
echo nasm isrs.o
gcc -Iinclude -c -o gdt.o gdt.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc gdt.o
gcc -Iinclude -c -o mem.o mem.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc mem.o
gcc -Iinclude -c -o screen.o screen.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc screen.o
gcc -Iinclude -c -o io.o io.c -fno-builtin -Wall  -fomit-frame-pointer -fno-stack-protector
echo gcc io.o
gcc -Iinclude -c -o main.o main.c -fno-builtin -Wall -nostdlib -fomit-frame-pointer -fno-stack-protector
echo gcc main.o
gcc -Iinclude -c -o idt.o idt.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc idt.o
gcc -Iinclude -c -o irq_handler.o irq.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc irq_handler.o
gcc -Iinclude -c -o isr.o isr.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc isr.o
gcc -Iinclude -c -o keyboard.o keyboard.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc keyboard.o
gcc -Iinclude -c -o memory.o memory.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc memory.o
gcc -Iinclude -c -o panic.o panic.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc panic.o
gcc -Iinclude -c -o malloc.o malloc.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc malloc.o
gcc -Iinclude -c -o timer.o timer.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc timer.o
gcc -Iinclude -c -o bit.o bit.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc timer.o
gcc -Iinclude -c -o floppy.o floppy.c -fno-builtin -Wall -fomit-frame-pointer -fno-stack-protector
echo gcc timer.o
ld -T link.ld -o kernel start.o mem.o screen.o main.o io.o gdt.o gdt_flush.o idt.o irq.o isr.o isrs.o keyboard.o irq_handler.o memory.o panic.o malloc.o timer.o bit.o floppy.o
echo Kernel linked!
