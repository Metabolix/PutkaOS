
CC=gcc
CFLAGS=-Wall -fomit-frame-pointer -fno-builtin -fno-stack-protector -nostdinc -I../include -O2

ASM=nasm
ASMFLAGS=-f elf

ASM_SOURCES=start.s gdt_asm.s irq_asm.s isrs.s bit.s regs_asm.s thread_asm.s
C_SOURCES=gdt.c io.c isr.c main.c mem.c panic.c timer.c floppy.c idt.c irq.c keyboard.c malloc.c memory.c screen.c string.c kprintf.c blockdev.c thread.c process.c regs.c

ASM_OBJS=$(ASM_SOURCES:.s=.o)
C_OBJS=$(C_SOURCES:.c=.o)

OBJS=$(ASM_OBJS) $(C_OBJS)

all: $(ASM_OBJS) $(C_OBJS)
	ld -T link.ld -o ../kernel $(ASM_OBJS) $(C_OBJS)

$(ASM_OBJS): %.o: %.s
	$(ASM) $(ASMFLAGS) $< -o $@

$(C_OBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(ASM_OBJS) $(C_OBJS)

