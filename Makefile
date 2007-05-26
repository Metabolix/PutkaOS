
CC=gcc
CFLAGS=-Wall -ffreestanding -fno-stack-protector -nostdinc -I./include -s -m32 -pedantic -std=c99
#-pedantic -std=c99 -Werror
CFLAGS_OPTI=

ASM=nasm
ASMFLAGS=-f elf

ASM_SRC=start.s gdt_asm.s irq_asm.s isrs.s bit.s thread_asm.s io.s read_cmos.s misc_asm.s

C_SRC_STDROUTINES=string.c mem.c ctype.c

C_SRC_MEM=memory.c malloc.c

C_SRC_MULTITASK=thread.c process.c

# Storage
C_SRC_BLOCKDEV_1=floppy.c ide.c hdd.c
C_SRC_BLOCKDEV=$(addprefix blockdev/,$(C_SRC_BLOCKDEV_1))
C_SRC_BLOCKDEV_OPT_1=blockdev.c
C_SRC_BLOCKDEV_OPT=$(addprefix blockdev/,$(C_SRC_BLOCKDEV_OPT_1))

# Storage
C_SRC_DEVICES_1=devmanager.c ports/serial.c $(C_SRC_BLOCKDEV)
C_SRC_DEVICES=$(addprefix devices/,$(C_SRC_DEVICES_1))
C_SRC_DEVICES_OPT_1=specialdevs.c specialdevs_func.c $(C_SRC_BLOCKDEV_OPT)
C_SRC_DEVICES_OPT=$(addprefix devices/,$(C_SRC_DEVICES_OPT_1))

# File system
C_SRC_FS_1=mount.c filesystem.c pseudofsdriver.c file.c dir.c fat.c fat16.c ext2.c
C_SRC_FS=$(addprefix filesys/,$(C_SRC_FS_1))

# Misc
C_SRC_OTHER=gdt.c isr.c main.c panic.c idt.c irq.c keyboard.c screen.c regs.c spinlock.c lcdscreen.c
#mouse.c
C_SRC_OTHER_OPT=int64.c timer.c kprintf.c sh.c sh_komennot.c time.c endian.c list.c

C_SRC=$(C_SRC_MEM) $(C_SRC_MULTITASK) $(C_SRC_DEVICES) $(C_SRC_OTHER)
C_SRC_OPT=$(C_SRC_OTHER_OPT) $(C_SRC_STDROUTINES) $(C_SRC_FS) $(C_SRC_DEVICES_OPT)

ASM_SOURCES=$(addprefix src/,$(ASM_SRC))
C_SOURCES=$(addprefix src/,$(C_SRC))
C_SOURCES_OPT=$(addprefix src/,$(C_SRC_OPT))

ASM_OBJS=$(ASM_SOURCES:.s=.o)
C_OBJS=$(C_SOURCES:.c=.o)
C_OBJS_OPT=$(C_SOURCES_OPT:.c=.o)
OBJS=$(ASM_OBJS) $(C_OBJS)

LDFLAGS=--oformat=elf32-i386 -melf_i386

all: $(ASM_OBJS) $(C_OBJS) $(C_OBJS_OPT)
	ld -T link.ld $(LDFLAGS) -o ./kernel $(ASM_OBJS) $(C_OBJS) $(C_OBJS_OPT)

$(ASM_OBJS): %.o: %.s
	$(ASM) $(ASMFLAGS) $< -o $@

$(C_OBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

$(C_OBJS_OPT): %.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_OPT) $< -c -o $@

clean:
	rm -f $(ASM_OBJS) $(C_OBJS) $(C_OBJS_OPT)

