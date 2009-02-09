LINKER_SCRIPT := ./misc/link.ld
KERNEL_FILE := ./kernel

LDFLAGS := --oformat=elf32-i386 -melf_i386

ASM := nasm
ASMFLAGS := -f elf -O1

CC := gcc
CFLAG_NO_STACK_PROTECTOR := $(shell echo "int main(void) {return 0;}" | gcc -fno-stack-protector -x c - -o /dev/null && echo "-fno-stack-protector")
#CFLAGS_41 := -V 4.1 -fno-stack-protector
#CFLAGS_40 := -V 4.0
#CFLAGS_34 := -V 3.4
CFLAGS_ALL := $(CFLAG_NO_STACK_PROTECTOR) -Wall -ffreestanding -nostdinc -I./putka-clib/include -g -m32 -pedantic -std=c99 -Os
#-pedantic -std=c99 -Werror

CFLAGS_KERNEL := $(CFLAGS_ALL) -I./include
CFLAGS_OPTI := -O
CFLAGS_STD_C := $(CFLAGS_ALL) -O2
ASMFLAGS_STD_C := $(ASMFLAGS)

KERNEL_DIRS := \
	memory \
	multitasking \
	syscall \
	devices \
		devices/blockdev \
		devices/ports \
		devices/display \
			devices/display/text \
	filesys \
		filesys/minix \
		filesys/fat \
		filesys/ext2 \
		filesys/iso9660 \
	utils

STD_C_DIRS := pos

ASM_SRC := start.asm irq.asm isrs.asm bit.asm io.asm read_cmos.asm misc_asm.asm syscall/syscall.asm

CO_SRC_STDROUTINES := endian.c list.c

CO_SRC_MEM_1 := init.c memory.c malloc.c swap.c pagefault.c
CO_SRC_MEM := $(addprefix memory/,$(CO_SRC_MEM_1))

C_SRC_MULTITASK_1 := multitasking.c thread.c process.c scheduler.c
C_SRC_MULTITASK := $(addprefix multitasking/,$(C_SRC_MULTITASK_1))

# Storage
C_SRC_BLOCKDEV_1 := floppy.c ide.c hdd.c
C_SRC_BLOCKDEV := $(addprefix blockdev/,$(C_SRC_BLOCKDEV_1))
CO_SRC_BLOCKDEV_1 := blockdev.c
CO_SRC_BLOCKDEV := $(addprefix blockdev/,$(CO_SRC_BLOCKDEV_1))

# Display
C_SRC_DISPLAY_1 :=
C_SRC_DISPLAY := $(addprefix display/,$(C_SRC_DISPLAY_1))
CO_SRC_DISPLAY_1 := text/lcdscreen.c text/pc_display.c
CO_SRC_DISPLAY := $(addprefix display/,$(CO_SRC_DISPLAY_1))

# Devices
C_SRC_DEVICES_1 := devmanager.c ports/serial.c $(C_SRC_BLOCKDEV) $(C_SRC_DISPLAY)
C_SRC_DEVICES := $(addprefix devices/,$(C_SRC_DEVICES_1))
CO_SRC_DEVICES_1 := specialdevs.c specialdevs_func.c $(CO_SRC_BLOCKDEV) $(CO_SRC_DISPLAY)
CO_SRC_DEVICES := $(addprefix devices/,$(CO_SRC_DEVICES_1))

# File system
CO_SRC_FS_MINIX_1 := minix.c zones.c fileutils.c maps.c
CO_SRC_FS_MINIX := $(addprefix minix/,$(CO_SRC_FS_MINIX_1))

CO_SRC_FS_FAT_1 := fat.c fat16.c
CO_SRC_FS_FAT := $(addprefix fat/,$(CO_SRC_FS_FAT_1))

CO_SRC_FS_EXT2_1 := ext2.c
CO_SRC_FS_EXT2 := $(addprefix ext2/,$(CO_SRC_FS_EXT2_1))

CO_SRC_FS_ISO9660_1 := iso9660.c
CO_SRC_FS_ISO9660 := $(addprefix iso9660/,$(CO_SRC_FS_ISO9660_1))

CO_SRC_FS_1 := mount.c filesystem.c file.c dir.c fileutils.c $(CO_SRC_FS_MINIX) $(CO_SRC_FS_FAT) $(CO_SRC_FS_EXT2) $(CO_SRC_FS_ISO9660)
CO_SRC_FS := $(addprefix filesys/,$(CO_SRC_FS_1))

CO_SRC_SYSCALL_1 := init.c
CO_SRC_SYSCALL := $(addprefix syscall/,$(CO_SRC_SYSCALL_1))

# Core
C_SRC_CORE := gdt.c isr.c main.c panic.c idt.c irq.c keyboard.c spinlock.c vt.c doublefault.c
CO_SRC_CORE := timer.c sh.c sh_komennot.c exec.c

# Utils
CO_SRC_UTILS_1 := texteditor.c
CO_SRC_UTILS := $(addprefix utils/,$(CO_SRC_UTILS_1))

# Misc
C_SRC_MISC :=
CO_SRC_MISC :=

C_SRC := $(C_SRC_CORE)  $(C_SRC_MEM)  $(C_SRC_MULTITASK) $(C_SRC_DEVICES) $(C_SRC_MISC)
CO_SRC := $(CO_SRC_CORE) $(CO_SRC_MEM) $(CO_SRC_STDROUTINES) $(CO_SRC_FS) $(CO_SRC_DEVICES) $(CO_SRC_SYSCALL) $(CO_SRC_UTILS) $(CO_SRC_MISC)

# file.c => build/file.c.o
ASM_OBJS := $(addsuffix .o,$(addprefix ./build/,$(ASM_SRC)))
C_OBJS := $(addsuffix .o,$(addprefix ./build/,$(C_SRC)))
CO_OBJS := $(addsuffix .o,$(addprefix ./build/,$(CO_SRC)))

KERNEL_OBJS := $(ASM_OBJS) $(C_OBJS) $(CO_OBJS)
KERNEL_DEPS := $(addprefix ./build/,$(C_SRC) $(CO_SRC))

STD_C_ASM_SRC := string.asm math.asm pos/mksyscall.asm build_tweaks.asm
STD_C_C_SRC := int64.c string.c stdlib.c stdio.fmt.c stdio.xprintf.c stdio.c inttypes.c math.c time.c ctype.c pos/time.c pos/file.c
STD_C_ASM_OBJS := $(addsuffix .o,$(addprefix ./build/putka-clib/,$(STD_C_ASM_SRC)))
STD_C_C_OBJS := $(addsuffix .o,$(addprefix ./build/putka-clib/,$(STD_C_C_SRC)))
STD_C_OBJS := $(STD_C_C_OBJS) $(STD_C_ASM_OBJS)
STD_C_DEPS := $(addprefix ./build/,$(STD_C_C_SRC))

RTL_SYSCALL_C_SRC := pos/syscalls.c
RTL_SYSCALL_C_OBJ := $(addsuffix .o,$(addprefix ./build/putka-clib/,$(RTL_SYSCALL_C_SRC)))
RTL_START_ASM := pos/start.asm
RTL_START_ASM_OBJ := $(addsuffix .o,$(addprefix ./build/putka-clib/,$(RTL_START_ASM)))
RTL_SYSCALL_DEPS := $(addprefix ./build/,$(RTL_SYSCALL_C_SRC))

RTL_FINAL_OBJ := ./build/rtl.o

ALL_C_SRC := $(STD_C_C_SRC) $(RTL_SYSCALL_C_SRC) $(C_SRC) $(CO_SRC)
ALL_C_DEPS := $(addprefix ./build/,$(ALL_C_SRC))

all: kernel rtl
std_c: std_c_builddirs $(STD_C_OBJS)
kernel: kernel_builddirs $(KERNEL_OBJS) std_c
rtl: std_c $(RTL_SYSCALL_C_OBJ) $(RTL_START_ASM_OBJ) $(RTL_FINAL_OBJ)

builddirs: kernel_builddirs std_c_builddirs
clean: clean_kernel clean_std_c clean_rtl clean_deps

kernel_builddirs:
	@mkdir -p ./build $(addprefix ./build/,$(KERNEL_DIRS)) || echo "mkdir failed!"

kernel:
	@echo Linking kernel...
	@ld -T $(LINKER_SCRIPT) $(LDFLAGS) -o $(KERNEL_FILE) $(KERNEL_OBJS) $(STD_C_OBJS)
	@echo Kernel linked!

std_c_builddirs:
	@mkdir -p ./build ./build/putka-clib $(addprefix ./build/putka-clib/,$(STD_C_DIRS)) || echo "mkdir failed!"

rtl:
	@echo "#!/bin/sh" > link.sh
	@echo "ld -melf_i386 --oformat=binary -Ttext 0x20000000 \`dirname \$$0\`/$(RTL_FINAL_OBJ) \$$@ " >> link.sh
	@chmod +x link.sh
	@echo "Ajonaikainen kirjasto $(RTL_FINAL_OBJ) ja skripti link.sh luotu."
	@echo
	@echo "Ohje omaa ohjelmaa varten: (-m32 mukaan amd64:lla)"
	@echo "gcc -ffreestanding -nostdinc -I./putka-clib/include -std=c99 -c -o a.o a.c"
	@echo "gcc ... -c -o b.o b.c"
	@echo "./link.sh a.o b.o -o ohjelma.bin"

$(RTL_FINAL_OBJ): $(RTL_START_ASM_OBJ) $(RTL_SYSCALL_C_OBJ) std_c
	@ld $(RTL_START_ASM_OBJ) $(RTL_SYSCALL_C_OBJ) $(STD_C_OBJS) $(LDFLAGS) -r -o $(RTL_FINAL_OBJ)

#$(ASM_OBJS): builddirs
#$(C_OBJS): builddirs
#$(CO_OBJS): builddirs


$(KERNEL_DEPS): ./build/%: src/%
	@echo [DEPS] $@
	@$(CC) $(CFLAGS_KERNEL) -MM $< -MF $@ -MT $@.o

-include $(KERNEL_DEPS)

$(ASM_OBJS): ./build/%.o: src/%
	@echo [ASM] $@
	@$(ASM) $(ASMFLAGS) $< -o $@

$(C_OBJS): ./build/%.o: src/%
	@echo [CC] $@
	@$(CC) $(CFLAGS_KERNEL) $< -c -o $@

$(CO_OBJS): ./build/%.o: src/%
	@echo [CC] $@
	@$(CC) $(CFLAGS_KERNEL) $(COFLAGS) $< -c -o $@


$(STD_C_DEPS): ./build/%: src/%
	@echo [DEPS] $@
	@$(CC) $(CFLAGS_STD_C) -MM $< -MF $@ -MT $@.o

-include $(STD_C_DEPS)

$(STD_C_ASM_OBJS): ./build/putka-clib/%.o: putka-clib/src/%
	@echo [ASM] $@
	@$(ASM) $(ASMFLAGS_STD_C) $< -o $@

$(STD_C_C_OBJS): ./build/putka-clib/%.o: putka-clib/src/%
	@echo [CC] $@
	@$(CC) $(CFLAGS_STD_C) $< -c -o $@


$(RTL_SYSCALL_DEPS): ./build/%: src/%
	@echo [DEPS] $@
	@$(CC) $(CFLAGS_STD_C) -MM $< -MF $@ -MT $@.o

-include $(KERNEL_DEPS)

$(RTL_SYSCALL_C_OBJ): ./build/putka-clib/%.o: putka-clib/src/%
	@echo [CC] $@
	@$(CC) $(CFLAGS_STD_C) $< -c -o $@

$(RTL_START_ASM_OBJ): ./build/putka-clib/%.o: putka-clib/src/%
	@echo [ASM] $@
	@$(ASM) $(ASMFLAGS_STD_C) $< -o $@

clean_kernel: clean_std_c
	rm -f $(ASM_OBJS) $(CO_OBJS) $(C_OBJS)
clean_rtl: clean_std_c
	rm -f $(RTL_START_ASM_OBJ) $(RTL_SYSCALL_C_OBJ) $(RTL_FINAL_OBJ)
clean_std_c:
	rm -f $(STD_C_OBJS)
clean_deps:
	rm -f $(ALL_C_DEPS)
