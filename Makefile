
CC=gcc
CFLAGS=-Wall -fomit-frame-pointer -fno-builtin -nostdinc -I./include -s -m32

ASM=nasm
ASMFLAGS=-f elf

ASM_SOURCES=start.s gdt_asm.s irq_asm.s isrs.s bit.s thread_asm.s io.s
C_SOURCES=gdt.c isr.c main.c mem.c panic.c timer.c floppy.c idt.c irq.c keyboard.c memory.c screen.c string.c kprintf.c blockdev.c thread.c process.c regs.c malloc.c

ASM_SRC=$(addprefix src/,$(ASM_SOURCES))
C_SRC=$(addprefix src/,$(C_SOURCES))

ASM_OBJS=$(ASM_SRC:.s=.o)
C_OBJS=$(C_SRC:.c=.o)
OBJS=$(ASM_OBJS) $(C_OBJS)

LDFLAGS=--format=elf32-i386 --oformat=elf32-i386

all: $(ASM_OBJS) $(C_OBJS)
	ld -T link.ld $(LDFLAGS) -o ./kernel $(ASM_OBJS) $(C_OBJS)

$(ASM_OBJS): %.o: %.s
	$(ASM) $(ASMFLAGS) $< -o $@

$(C_OBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(ASM_OBJS) $(C_OBJS)

