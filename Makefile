
CC=gcc
CFLAGS=-Wall -fomit-frame-pointer -fno-builtin -nostdinc -I./include -s -m32
CFLAGS_OPTI=-O

ASM=nasm
ASMFLAGS=-f elf

ASM_SOURCES=start.s gdt_asm.s irq_asm.s isrs.s bit.s thread_asm.s io.s

C_SOURCES=memory.c malloc.c thread.c process.c
#C_SOURCES=gdt.c isr.c main.c mem.c panic.c timer.c floppy.c idt.c irq.c keyboard.c memory.c screen.c string.c kprintf.c blockdev.c thread.c process.c regs.c malloc.c ext2.c mount.c devmanager.c filesystem.c pseudofsdriver.c stdio.c spinlock.c sh.c sh_komennot.c lcdscreen.c
# Optimoitavat koodit
C_SOURCES_OPTIMIZE=gdt.c isr.c main.c panic.c timer.c floppy.c idt.c irq.c keyboard.c mem.c screen.c string.c kprintf.c blockdev.c regs.c ext2.c mount.c devmanager.c filesystem.c pseudofsdriver.c stdio.c spinlock.c sh.c sh_komennot.c lcdscreen.c


ASM_SRC=$(addprefix src/,$(ASM_SOURCES))
C_SRC=$(addprefix src/,$(C_SOURCES))
C_SRC_OPTI=$(addprefix src/,$(C_SOURCES_OPTIMIZE))

ASM_OBJS=$(ASM_SRC:.s=.o)
C_OBJS=$(C_SRC:.c=.o)
C_OBJS_OPTI=$(C_SRC_OPTI:.c=.o)
OBJS=$(ASM_OBJS) $(C_OBJS)

LDFLAGS=--oformat=elf32-i386

all: $(ASM_OBJS) $(C_OBJS) $(C_OBJS_OPTI)
	ld -T link.ld $(LDFLAGS) -o ./kernel $(ASM_OBJS) $(C_OBJS) $(C_OBJS_OPTI)

$(ASM_OBJS): %.o: %.s
	$(ASM) $(ASMFLAGS) $< -o $@

$(C_OBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

$(C_OBJS_OPTI): %.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_OPTI) $< -c -o $@

clean:
	rm -f $(ASM_OBJS) $(C_OBJS) $(C_OBJS_OPTI)

