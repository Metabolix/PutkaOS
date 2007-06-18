
CC=gcc
CFLAGS_41=-V 4.1 -fno-stack-protector
CFLAGS_40=-V 4.0
CFLAGS_34=-V 3.4
CFLAGS_ALL=-Wall -ffreestanding -nostdinc -I./include -s -m32 -pedantic -std=c99
#-pedantic -std=c99 -Werror

CFLAGS=$(CFLAGS_ALL)
CFLAGS_OPTI=

DIRS=devices devices/blockdev devices/ports filesys

ASM=nasm
ASMFLAGS=-f elf

ASM_SRC=start.asm gdt.asm irq.asm isrs.asm bit.asm thread.asm io.asm read_cmos.asm misc.asm math.asm sprintf.asm build_tweaks.asm

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

# file.c => build/file.c.o
ASM_OBJS=$(addsuffix .o,$(addprefix build/,$(ASM_SRC)))
C_OBJS=$(addsuffix .o,$(addprefix build/,$(C_SRC)))
CO_OBJS=$(addsuffix .o,$(addprefix build/,$(CO_SRC)))

OBJS=$(ASM_OBJS) $(C_OBJS) $(CO_OBJS)

LDFLAGS=--oformat=elf32-i386 -melf_i386

all: builddirs $(OBJS)
	@echo Linking kernel...
	@ld -T link.ld $(LDFLAGS) -o ./kernel $(OBJS)
	@echo Kernel linked!

builddirs:
	@mkdir -p build $(addprefix build/,$(DIRS)) || echo "mkdir failed!"

#$(ASM_OBJS): builddirs
#$(C_OBJS): builddirs
#$(CO_OBJS): builddirs

$(ASM_OBJS): build/%.o: src/%
	@echo [ASM] $@
	@$(ASM) $(ASMFLAGS) $< -o $@

$(C_OBJS): build/%.o: src/%
	@echo [CC] $@
	@$(CC) $(CFLAGS) $< -c -o $@

$(CO_OBJS): build/%.o: src/%
	@echo [CC] $@
	@$(CC) $(CFLAGS) $(COFLAGS) $< -c -o $@

clean:
	rm -f $(ASM_OBJS) $(CO_OBJS) $(C_OBJS)

