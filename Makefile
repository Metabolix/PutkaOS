
CC=gcc
CFLAGS_41=-V 4.1 -fno-stack-protector
CFLAGS_40=-V 4.0
CFLAGS_34=-V 3.4
CFLAGS_ALL=-Wall -ffreestanding -nostdinc -I./include -s -m32 -pedantic -std=c99
CFLAGS=$(CFLAGS_ALL)

DIRS=devices/{,blockdev,ports} filesys
DIR_PREFIX=build/{c,asm}/

#-pedantic -std=c99 -Werror
CFLAGS_OPTI=

ASM=nasm
ASMFLAGS=-f elf

ASM_SRC=start.s gdt_asm.s irq_asm.s isrs.s bit.s thread_asm.s io.s read_cmos.s misc_asm.s math_asm.s sprintf_asm.s

CO_SRC_STDROUTINES=string.c mem.c ctype.c
CO_SRC_MEM=memory.c malloc.c

C_SRC_MULTITASK=thread.c process.c

# Storage
C_SRC_BLOCKDEV_1=floppy.c ide.c hdd.c
C_SRC_BLOCKDEV=$(addprefix blockdev/,$(C_SRC_BLOCKDEV_1))
CO_SRC_BLOCKDEV_1=blockdev.c
CO_SRC_BLOCKDEV=$(addprefix blockdev/,$(CO_SRC_BLOCKDEV_1))

# Storage
C_SRC_DEVICES_1=devmanager.c ports/serial.c $(C_SRC_BLOCKDEV)
C_SRC_DEVICES=$(addprefix devices/,$(C_SRC_DEVICES_1))
CO_SRC_DEVICES_1=specialdevs.c specialdevs_func.c $(CO_SRC_BLOCKDEV)
CO_SRC_DEVICES=$(addprefix devices/,$(CO_SRC_DEVICES_1))

# File system
CO_SRC_FS_1=mount.c filesystem.c pseudofsdriver.c file.c dir.c fat.c fat16.c ext2.c minix.c
CO_SRC_FS=$(addprefix filesys/,$(CO_SRC_FS_1))

# Misc
C_SRC_OTHER=gdt.c isr.c main.c panic.c idt.c irq.c keyboard.c screen.c regs.c spinlock.c lcdscreen.c syscall.c
#mouse.c
CO_SRC_OTHER=int64.c timer.c kprintf.c sh.c sh_komennot.c time.c endian.c list.c fprintf.c sprintf.c math.c

C_SRC=$(C_SRC_MEM) $(C_SRC_MULTITASK) $(C_SRC_DEVICES) $(C_SRC_OTHER)
CO_SRC=$(CO_SRC_MEM) $(CO_SRC_OTHER) $(CO_SRC_STDROUTINES) $(CO_SRC_FS) $(CO_SRC_DEVICES)

ASM_SOURCES=$(addprefix build/asm/,$(ASM_SRC))
C_SOURCES=$(addprefix build/c/,$(C_SRC))
CO_SOURCES=$(addprefix build/c/,$(CO_SRC))

ASM_OBJS=$(ASM_SOURCES:.s=.o)
C_OBJS=$(C_SOURCES:.c=.o)
CO_OBJS=$(CO_SOURCES:.c=.o)

OBJS=$(ASM_OBJS) $(C_OBJS) $(CO_OBJS)

LDFLAGS=--oformat=elf32-i386 -melf_i386

all: builddirs $(ASM_OBJS) $(C_OBJS) $(CO_OBJS)
	@echo Linking kernel...
	@ld -T link.ld $(LDFLAGS) -o ./kernel $(ASM_OBJS) $(CO_OBJS) $(C_OBJS)
	@echo Kernel linked!

builddirs:
	@mkdir -p $(addprefix $(DIR_PREFIX),$(DIRS)) || echo "mkdir failed!"
#$(ASM_OBJS): builddirs
#$(C_OBJS): builddirs
#$(CO_OBJS): builddirs

$(ASM_OBJS): build/asm/%.o: src/%.s
	@echo [ASM] $@
	@$(ASM) $(ASMFLAGS) $< -o $@

$(C_OBJS): build/c/%.o: src/%.c
	@echo [CC] $@
	@$(CC) $(CFLAGS) $< -c -o $@

$(CO_OBJS): build/c/%.o: src/%.c
	@echo [CC] $@
	@$(CC) $(CFLAGS) $(COFLAGS) $< -c -o $@

clean:
	rm -f $(ASM_OBJS) $(CO_OBJS) $(C_OBJS)

