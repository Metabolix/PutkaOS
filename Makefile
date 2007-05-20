
CC=gcc
CFLAGS=-Wall -ffreestanding -fno-stack-protector -nostdinc -I./include -s -m32 -pedantic -std=c99
#-pedantic -std=c99 -Werror
CFLAGS_OPTI=

ASM=nasm
ASMFLAGS=-f elf

ASM_SOURCES=start.s gdt_asm.s irq_asm.s isrs.s bit.s thread_asm.s io.s read_cmos.s misc_asm.s

C_SOURCES_STDROUTINES=string.c mem.c

C_SOURCES_MEM=memory.c malloc.c

C_SOURCES_MULTITASK=thread.c process.c

# Storage
C_SOURCES_BLOCKDEV_1=blockdev.c floppy.c ide.c hdd.c
C_SOURCES_BLOCKDEV=$(addprefix block/,$(C_SOURCES_BLOCKDEV_1))

# Storage
C_SOURCES_DEVICES_1=devmanager.c $(C_SOURCES_BLOCKDEV)
C_SOURCES_DEVICES=$(addprefix devices/,$(C_SOURCES_DEVICES_1))

# File system
C_SOURCES_FS_1=mount.c filesystem.c pseudofsdriver.c file.c dir.c fat.c fat16.c ext2.c
C_SOURCES_FS=$(addprefix filesys/,$(C_SOURCES_FS_1))

# Misc
C_SOURCES_OTHER=gdt.c isr.c main.c panic.c idt.c irq.c keyboard.c screen.c regs.c spinlock.c lcdscreen.c
#mouse.c
C_SOURCES_OTHER_OPT=int64.c timer.c kprintf.c sh.c sh_komennot.c time.c endian.c list.c

C_SOURCES=$(C_SOURCES_MEM) $(C_SOURCES_MULTITASK) $(C_SOURCES_DEVICES) $(C_SOURCES_OTHER)
C_SOURCES_OPTIMIZE=$(C_SOURCES_OTHER_OPT) $(C_SOURCES_STDROUTINES) $(C_SOURCES_FS)

ASM_SRC=$(addprefix src/,$(ASM_SOURCES))
C_SRC=$(addprefix src/,$(C_SOURCES))
C_SRC_OPTI=$(addprefix src/,$(C_SOURCES_OPTIMIZE))

ASM_OBJS=$(ASM_SRC:.s=.o)
C_OBJS=$(C_SRC:.c=.o)
C_OBJS_OPTI=$(C_SRC_OPTI:.c=.o)
OBJS=$(ASM_OBJS) $(C_OBJS)

LDFLAGS=--oformat=elf32-i386 -melf_i386

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

