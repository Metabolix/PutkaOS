LINKER_SCRIPT=misc/link.ld
KERNEL_FILE=kernel

LDFLAGS=--oformat=elf32-i386 -melf_i386

ASM=nasm
ASMFLAGS=-f elf

CC=gcc
CFLAGS_41=-V 4.1 -fno-stack-protector
CFLAGS_40=-V 4.0
CFLAGS_34=-V 3.4
CFLAGS_ALL=-Wall -ffreestanding -nostdinc -I./include -g -m32 -pedantic -std=c99
#-pedantic -std=c99 -Werror

CFLAGS=$(CFLAGS_41) $(CFLAGS_ALL)
CFLAGS_OPTI=-O2

DIRS= \
	memory \
	multitasking \
	devices \
		devices/blockdev devices/ports devices/display \
			devices/display/text \
	filesys \
		filesys/minix filesys/fat filesys/ext2 \
	utils

ASM_SRC=start.asm irq.asm isrs.asm bit.asm multitasking/thread.asm io.asm read_cmos.asm misc_asm.asm math.asm build_tweaks.asm syscall.asm string.asm

CO_SRC_STDROUTINES=string.c ctype.c int64.c endian.c list.c fprintf.c math.c xprintf_xscanf.c

CO_SRC_MEM_1=init.c memory.c malloc.c swap.c pagefault.c
CO_SRC_MEM=$(addprefix memory/,$(CO_SRC_MEM_1))

C_SRC_MULTITASK_1=multitasking.c thread.c process.c
C_SRC_MULTITASK=$(addprefix multitasking/,$(C_SRC_MULTITASK_1))

# Storage
C_SRC_BLOCKDEV_1=floppy.c ide.c hdd.c
C_SRC_BLOCKDEV=$(addprefix blockdev/,$(C_SRC_BLOCKDEV_1))
CO_SRC_BLOCKDEV_1=blockdev.c
CO_SRC_BLOCKDEV=$(addprefix blockdev/,$(CO_SRC_BLOCKDEV_1))

# Display
C_SRC_DISPLAY_1=
C_SRC_DISPLAY=$(addprefix display/,$(C_SRC_DISPLAY_1))
CO_SRC_DISPLAY_1=text/lcdscreen.c text/pc_display.c
CO_SRC_DISPLAY=$(addprefix display/,$(CO_SRC_DISPLAY_1))

# Devices
C_SRC_DEVICES_1=devmanager.c ports/serial.c $(C_SRC_BLOCKDEV) $(C_SRC_DISPLAY)
C_SRC_DEVICES=$(addprefix devices/,$(C_SRC_DEVICES_1))
CO_SRC_DEVICES_1=specialdevs.c specialdevs_func.c $(CO_SRC_BLOCKDEV) $(CO_SRC_DISPLAY)
CO_SRC_DEVICES=$(addprefix devices/,$(CO_SRC_DEVICES_1))

# File system
CO_SRC_FS_MINIX_1=minix.c zones.c fileutils.c maps.c
CO_SRC_FS_MINIX=$(addprefix minix/,$(CO_SRC_FS_MINIX_1))

CO_SRC_FS_FAT_1=fat.c fat16.c
CO_SRC_FS_FAT=$(addprefix fat/,$(CO_SRC_FS_FAT_1))

CO_SRC_FS_EXT2_1=ext2.c
CO_SRC_FS_EXT2=$(addprefix ext2/,$(CO_SRC_FS_EXT2_1))

CO_SRC_FS_1=mount.c filesystem.c file.c dir.c fileutils.c $(CO_SRC_FS_MINIX) $(CO_SRC_FS_FAT) $(CO_SRC_FS_EXT2)
CO_SRC_FS=$(addprefix filesys/,$(CO_SRC_FS_1))

# Core
C_SRC_CORE=gdt.c isr.c main.c panic.c idt.c irq.c keyboard.c regs.c spinlock.c syscall.c vt.c screen.c
CO_SRC_CORE=timer.c kprintf.c sh.c sh_komennot.c time.c

# Utils
CO_SRC_UTILS_1=texteditor.c
CO_SRC_UTILS=$(addprefix utils/,$(CO_SRC_UTILS_1))

# Misc
C_SRC_MISC=
CO_SRC_MISC=

C_SRC= $(C_SRC_CORE)  $(C_SRC_MEM)  $(C_SRC_MULTITASK) $(C_SRC_DEVICES) $(C_SRC_MISC)
CO_SRC=$(CO_SRC_CORE) $(CO_SRC_MEM) $(CO_SRC_STDROUTINES) $(CO_SRC_FS) $(CO_SRC_DEVICES) $(CO_SRC_UTILS) $(CO_SRC_MISC)

# file.c => build/file.c.o
ASM_OBJS=$(addsuffix .o,$(addprefix build/,$(ASM_SRC)))
C_OBJS=$(addsuffix .o,$(addprefix build/,$(C_SRC)))
CO_OBJS=$(addsuffix .o,$(addprefix build/,$(CO_SRC)))

OBJS=$(ASM_OBJS) $(C_OBJS) $(CO_OBJS)

all: builddirs $(OBJS) link

link:
	@echo Linking kernel...
	@ld -T $(LINKER_SCRIPT) $(LDFLAGS) -o $(KERNEL_FILE) $(OBJS)
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

