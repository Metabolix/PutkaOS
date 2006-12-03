
CC=gcc
CFLAGS=-Wall -fomit-frame-pointer -fno-builtin -fno-stack-protector -nostdinc -I../include

ASM=nasm
ASMFLAGS=-f elf

ASM_SOURCES=start.s gdt_asm.s irq_asm.s isrs.s
C_SOURCES=bit.c gdt.c io.c isr.c main.c mem.c panic.c timer.c floppy.c idt.c irq.c keyboard.c malloc.c memory.c screen.c string.c kprintf.c blockdev.c

ASM_OBJS=$(ASM_SOURCES:.s=.o)
C_OBJS=$(C_SOURCES:.c=.o)

OBJS=$(ASM_OBJS) $(C_OBJS)

all: $(ASM_OBJS) $(C_OBJS)
	ld -T link.ld -o ../kernel $(ASM_OBJS) $(C_OBJS)

.c.o:
	$(CC) $(CFLAGS) $< -c -o $@

.s.o:
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm $(ASM_OBJS) $(C_OBJS)

